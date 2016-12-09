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
static const void* trusted_state; // Trusted debug registers state
static struct task_struct* task; // DR monitor main task

static int monitor_loop(void* data);

int start_dr_monitor(void) {
	int res;

	// Get number of available debug registers
	dr_count = count_drs();
	if (dr_count == 0) {
		printk(KERN_INFO "Debug registers monitor not needed\n");
		return 0;
	}
	
	// Allocate space for trusted state
	trusted_state = kmalloc(DR_STATE_SIZE * dr_count, GFP_KERNEL);
	if (!trusted_state) {
		printk(KERN_ERR "Unable to allocate kernel space for debug registers monitor\n");
		res = -ENOMEM;
		goto trusted_failed;
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
	kfree((void*)trusted_state);
trusted_failed:
	return res;
}

static int monitor_loop(void* data) {
	dump_dr_state();
	while(1) {
		// Check DR state
		check_dr_state(trusted_state);

		msleep(DR_MONITOR_INTERVAL);
		if (kthread_should_stop()) return 0;
	}
	return 0;
}

void handle_dr_detection(dr_detect_t* info) {
	printk(KERN_INFO "Change detected on DR#%u state\n", info->index);
	dump_dr_state();
	restore_dr_state(info);
}

void stop_dr_monitor(void) {
	if (dr_count > 0) {
		kthread_stop(task);
		kfree((void*)trusted_state);
		printk(KERN_INFO "Debug registers monitor stopped\n");
	}
}
