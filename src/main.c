#include <linux/kernel.h>
#include <linux/module.h>

#include "log.h"
#include "io_monitor.h"
#include "dr_monitor.h"
#include "map_monitor.h"

int __init init_module(void) {
	int res;

	if ( (res = start_io_monitor()) )
		goto io_failed;

	if ( (res = start_dr_monitor()) )
		goto dr_failed;

	if ( (res = start_map_monitor()) )
		goto map_failed;

	log_info("Ghostbuster started\n");
	return 0;

map_failed:
	stop_dr_monitor();
dr_failed:
	stop_io_monitor();
io_failed:
	return res;
}

void __exit cleanup_module() {
	stop_map_monitor();
	stop_dr_monitor();
	stop_io_monitor();
	log_info("Ghostbuster stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tu4st");
MODULE_DESCRIPTION("Ghostbuster in the PLC");
