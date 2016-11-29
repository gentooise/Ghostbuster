#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "io_conf.h"
#include "config_monitor.h"

volatile void** addrs; // I/O virtual addresses
const void* trusted_values; // Trusted values in I/O memory
void* current_values;
struct task_struct* task;

static int map_addrs(void);
static void unmap_addrs(int mapped);

static int monitor_loop(void* data) {
	unsigned b, offset;

	while (1) {
		// Check I/O blocks
		for (b = 0, offset = 0; b < io_conf->blocks; offset += io_conf->sizes[b++]) { // For each block
			// Read from I/O memory in an architecture-independent manner.
			check_addrs_in_block(addrs[b], trusted_values + offset, b);
		}

		if (kthread_should_stop()) return 0;
		msleep(MONITOR_INTERVAL);
	}
}

void handle_detection(detect_info_t* info) {
	printk(KERN_INFO "Change detected on address 0x%08lx [old value = %ld, new value = %ld]\n",
		(long)info->target, info->old_val, info->new_val);
	restore_value(info);
}

int start_config_monitor() {
	
	int res;
	void* vals;

	res = map_addrs();
	if (res) return res;

	// Allocate space for trusted values
	vals = kmalloc(io_conf->size, GFP_KERNEL);
	if (!vals) {
		printk(KERN_ERR "Unable to allocate kernel space for configuration monitor\n");
		unmap_addrs(io_conf->blocks);
		return -ENOMEM;
	}

	trusted_values = read_values(addrs, vals);

#ifdef DEBUG
	dump_values(addrs);
#endif

	// Start monitor task
	task = kthread_run(&monitor_loop, NULL, "monitor");
	if (IS_ERR((void*)task)) {
		printk(KERN_ERR "[RK] Creating thread... failed %ld\n", PTR_ERR((void*)task));
		unmap_addrs(io_conf->blocks);
		kfree(vals);
		return PTR_ERR((void*)task);
	}

	printk(KERN_INFO "Configuration monitor started\n");
	return 0;

}

void stop_config_monitor(void) {
	kthread_stop(task);
	unmap_addrs(io_conf->blocks);
	kfree(trusted_values);
	printk(KERN_INFO "Configuration monitor stopped\n");
}

static int map_addrs() {
	int i;

	addrs = kmalloc(sizeof(void*) * io_conf->blocks, GFP_KERNEL);
	if (!addrs) {
		printk(KERN_ERR "Unable to allocate kernel space for configuration monitor\n");
		return -ENOMEM;
	}

	for (i = 0; i < io_conf->blocks; i++) {
		addrs[i] = ioremap((phys_addr_t)io_conf->addrs[i], io_conf->sizes[i]);
		if (IS_ERR((void*)addrs[i])) {
			printk(KERN_ERR "Unable to map I/O address %08lx\n", (long)io_conf->addrs[i]);
			unmap_addrs(i);
			return PTR_ERR((void*)addrs[i]);
		}
	}

	return 0;
}

static void unmap_addrs(int mapped) {
	int i;

	for (i = 0; i < mapped; i++) {
		iounmap(addrs[i]);
	}

	kfree(addrs);
}
