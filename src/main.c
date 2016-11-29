#include <linux/kernel.h>
#include <linux/module.h>

#include "io_conf.h"
#include "config_monitor.h"

// Take physical I/O configuration from architecture specific struct.
const io_conf_t* io_conf = &phys_io_conf;

int __init init_module(void) {
	int res;

	res = start_config_monitor();
	if (res) return res;

	printk(KERN_INFO "Ghostbuster started\n");
	return 0;
}

void __exit cleanup_module() {
	stop_config_monitor();
	printk(KERN_INFO "Ghostbuster stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghostbuster in the PLC");
