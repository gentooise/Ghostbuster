#include <linux/kernel.h>
#include <linux/module.h>

#include "config_monitor.h"
#include "dr_monitor.h"

int __init init_module(void) {
	int res;

	res = start_config_monitor();
	if (res) goto config_failed;

	res = start_dr_monitor();
	if (res) goto dr_failed;

	printk(KERN_INFO "Ghostbuster started\n");
	return 0;

dr_failed:
	stop_config_monitor();
config_failed:
	return res;
}

void __exit cleanup_module() {
	stop_dr_monitor();
	stop_config_monitor();
	printk(KERN_INFO "Ghostbuster stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghostbuster in the PLC");
