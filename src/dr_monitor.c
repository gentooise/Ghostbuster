#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "dr_monitor.h"
#include "dr_conf.h"

unsigned dr_count;
const void* trusted_state; // Trusted debug registers state
volatile void* current_state; // Current debug registers state
static struct task_struct* task;

#ifdef DEBUG
static void dump_dr_values(void) {
	unsigned i, j;
	volatile void* state = (void*)trusted_state;
	printk(KERN_INFO "DR trusted state:\n");
	for (i = 0; i < dr_count; i++) {
		printk(KERN_INFO "\t#%u\t", i);
		for (j = 0; j < DR_STATE_SIZE; j++) {
			printk(KERN_CONT "%02x ", *((char*)(state+j)));
		}
		state += DR_STATE_SIZE;
		printk(KERN_CONT "\n");
	}

	state = (void*)current_state;
	printk(KERN_INFO "DR current state:\n");
	for (i = 0; i < dr_count; i++) {
		printk(KERN_INFO "\t#%u\t", i);
		for (j = 0; j < DR_STATE_SIZE; j++) {
			printk(KERN_CONT "%02x ", *((char*)(state+j)));
		}
		state += DR_STATE_SIZE;
		printk(KERN_CONT "\n");
	}
}
#endif

static int monitor_loop(void* data) {
	while(1) {
		check_dr_state(current_state, trusted_state);
#ifdef DEBUG
		dump_dr_values();
#endif
		msleep(DR_MONITOR_INTERVAL);
		if (kthread_should_stop()) return 0;
	}
	return 0;
}

void handle_dr_detection(dr_detect_t* info) {
	printk(KERN_INFO "Change detected on DR state\n");
	restore_dr_state(info);
}

/*
 * This monitor is responsible for protecting debug registers from malicious usage.
 * There are basically two ways to use debug registers:
 *  1. Kernel space
 *  2. User space
 * 
 * From kernel space, it is possible to modify debug registers by means of one of the following:
 *  - "register_wide_hw_breakpoint" kernel interface to register breakpoints or watchpoints;
 *  - direct debug register access through CPU instructions (depending on the architecture).
 * To protect debug registers from kernel space attack, the monitor checks their value
 * every DR_MONITOR_INTERVAL milliseconds, and eventually restores their trusted value back.
 * 
 * From user space, direct access to debug registers is forbidden in Linux,
 * so the only way to use them is through the ptrace user interface.
 * When a user process requests an hardware breakpoint, ptrace uses the following function:
 *  - "register_user_hw_breakpoint"
 * Upon request, the hardware breakpoint is bound to the task, so that the kernel automatically
 * writes the corresponding debug registers when the task is scheduled, and it resets them back
 * when the process is switched out from the CPU.
 * 
 * Due to this fact, the kernel wide monitor alone is not enough, because when this module is running
 * it is only capable of reading the kernel side debug register, while the user space debug
 * registers are stored inside the corresponding task_struct of the process and will be available only
 * during the process execution. Thus, to protect debug registers from user space access as well,
 * we need to modify the behaviour of register_user_hw_breakpoint. <- TODO
 */
int start_dr_monitor(void) {
	int res;

	// Get number of available debug registers
	dr_count = count_drs();
	if (dr_count == 0) {
		printk(KERN_INFO "Debug registers monitor not needed\n");
		return 0;
	}
	
	// Allocate space for trusted states
	trusted_state = kmalloc(DR_STATE_SIZE * dr_count, GFP_KERNEL);
	if (!trusted_state) {
		printk(KERN_ERR "Unable to allocate kernel space for debug registers monitor\n");
		res = -ENOMEM;
		goto trusted_failed;
	}

	// Allocate space for current states
	current_state = kmalloc(DR_STATE_SIZE * dr_count, GFP_KERNEL);
	if (!current_state) {
		printk(KERN_ERR "Unable to allocate kernel space for debug registers monitor\n");
		res = -ENOMEM;
		goto current_failed;
	}

	// Get DR trusted state
	get_dr_state((void*)trusted_state);

	// Start monitor task
	task = kthread_run(&monitor_loop, NULL, "dr_monitor");
	if (IS_ERR((void*)task)) {
		printk(KERN_ERR "Unable to create thread: %ld\n", PTR_ERR((void*)task));
		res = PTR_ERR((void*)task);
		goto task_failed;
	}

	printk(KERN_INFO "Debug registers monitor started\n");
	return 0;

task_failed:
	kfree((void*)current_state);
current_failed:
	kfree((void*)trusted_state);
trusted_failed:
	return res;
}

void stop_dr_monitor(void) {
	if (dr_count > 0) {
		kthread_stop(task);
		kfree((void*)trusted_state);
		printk(KERN_INFO "Debug registers monitor stopped\n");
	}
}
