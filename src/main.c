#include <linux/kernel.h>
#include <linux/module.h>

#include "io_monitor.h"
#include "dr_monitor.h"

int __init init_module(void) {
	int res;

	res = start_io_monitor();
	if (res) goto io_failed;

	res = start_dr_monitor();
	if (res) goto dr_failed;

	printk(KERN_INFO "Ghostbuster started\n");
	return 0;

dr_failed:
	stop_io_monitor();
io_failed:
	return res;
}

void __exit cleanup_module() {
	stop_dr_monitor();
	stop_io_monitor();
	printk(KERN_INFO "Ghostbuster stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghostbuster in the PLC");
