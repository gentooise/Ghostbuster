#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "dr_monitor.h"
#include "dr_conf.h"
#include "dr_debug.h"

static unsigned dr_count; // Number of available debug registers
static const void* volatile trusted_state; // Trusted debug registers state
static struct task_struct* task; // DR monitor main task
DEFINE_MUTEX(trusted_lock); // Mutex to protect trusted state

static int monitor_loop(void* data);
static void disable_user_dr_interface(void);
static void enable_user_dr_interface(void);

int start_dr_monitor(void) {
	int res;

	// Get number of available debug registers
	dr_count = count_drs();
	if (dr_count == 0) {
		log_info("DR monitor not needed\n");
		return 0;
	}
	
	// Allocate space for trusted state
	trusted_state = kmalloc(DR_STATE_SIZE * dr_count, GFP_KERNEL);
	if (!trusted_state) {
		log_err("Unable to allocate kernel space for DR monitor\n");
		res = -ENOMEM;
		goto trusted_failed;
	}

	// Disable user DR interface
	disable_user_dr_interface();

	// Get DR trusted state
	get_dr_state((void*)trusted_state);

	// Start monitor task
	task = kthread_run(&monitor_loop, NULL, "dr_monitor");
	if (IS_ERR((void*)task)) {
		log_err("Unable to create thread: %ld\n", PTR_ERR((void*)task));
		res = PTR_ERR((void*)task);
		goto task_failed;
	}

	log_info("DR monitor started\n");
	return 0;

task_failed:
	kfree((void*)trusted_state);
trusted_failed:
	return res;
}

static int monitor_loop(void* data) {
	dump_dr_state();
	while(1) {
		mutex_lock(&trusted_lock);
		// Check DR state
		check_dr_state(trusted_state);
		mutex_unlock(&trusted_lock);

		usleep_range(DR_MIN_RANGE(DR_MONITOR_INTERVAL), DR_MAX_RANGE(DR_MONITOR_INTERVAL));
		if (kthread_should_stop()) return 0;
	}
	return 0;
}

void handle_dr_detection(dr_detect_t* info) {
	log_info("Change detected on DR#%u state\n", info->index);
	dump_dr_state();
	restore_dr_state(info);
}

static char register_user_dr_old[REGISTER_USER_DR_SIZE];
static char modify_user_dr_old[MODIFY_USER_DR_SIZE];
static char unregister_dr_old[UNREGISTER_DR_SIZE];
static void *register_user_hw_breakpoint_addr, *modify_user_hw_breakpoint_addr, *unregister_hw_breakpoint_addr;

#define toggle_user_dr_interface(x, y) do {                                                                                      \
	patch_ktext(register_user_hw_breakpoint_addr, register_user_dr_ ## x, (void*)register_user_dr_ ## y, REGISTER_USER_DR_SIZE);  \
	patch_ktext(modify_user_hw_breakpoint_addr, modify_user_dr_ ## x, (void*)modify_user_dr_ ## y, MODIFY_USER_DR_SIZE);          \
	patch_ktext(unregister_hw_breakpoint_addr, unregister_dr_ ## x, (void*)unregister_dr_ ## y, UNREGISTER_DR_SIZE);              \
} while(0)

static void disable_user_dr_interface(void) {
	register_user_hw_breakpoint_addr = (void*) kallsyms_lookup_name("register_user_hw_breakpoint");
	modify_user_hw_breakpoint_addr = (void*) kallsyms_lookup_name("modify_user_hw_breakpoint");
	unregister_hw_breakpoint_addr = (void*) kallsyms_lookup_name("unregister_hw_breakpoint");
	toggle_user_dr_interface(new, old); // Patch kernel text with opcodes
}

static void enable_user_dr_interface(void) {
	toggle_user_dr_interface(old, new); // Revert patches
}

static void* set_dr(int pid, void* vaddr, dr_handler_t handler, unsigned type) {
	void* dr;
	mutex_lock(&trusted_lock);
	dr = __set_dr(pid, vaddr, handler, type);
	if (trusted_state) get_dr_state((void*)trusted_state);
	mutex_unlock(&trusted_lock);
	return dr;
}

void* set_read_dr(int pid, void* vaddr, dr_handler_t handler) {
	return set_dr(pid, vaddr, handler, HW_BREAKPOINT_R);
}

void* set_write_dr(int pid, void* vaddr, dr_handler_t handler) {
	return set_dr(pid, vaddr, handler, HW_BREAKPOINT_W);
}

void reset_dr(void* dr) {
	mutex_lock(&trusted_lock);
	__reset_dr(dr);
	if (trusted_state) get_dr_state((void*)trusted_state);
	mutex_unlock(&trusted_lock);
}

void stop_dr_monitor(void) {
	if (dr_count > 0) {
		kthread_stop(task);
		mutex_lock(&trusted_lock);
		kfree((void*)trusted_state);
		trusted_state = NULL;
		mutex_unlock(&trusted_lock);
		enable_user_dr_interface();
		log_info("DR monitor stopped\n");
	}
}
