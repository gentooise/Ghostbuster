#include <linux/kernel.h>
#include <linux/module.h>

#include "log.h"
#include "io_monitor.h"
#include "dr_monitor.h"
#include "map_monitor.h"

static int p_pid;
static char* vaddr_base;

// Needed information about the PLC runtime to protect against Pin Control Attack.
// We assume PLC runtime is already running. In order to remove this assumption,
// PLC runtime could be recognized through mapping requests, by assuming that the
// system is safe when the PLC runtime is started. In this case it could be recognized
// from its process name by the system, without needing some form of authentication.
module_param(p_pid, int, 0);
MODULE_PARM_DESC(p_pid, "PLC runtime PID");
module_param(vaddr_base, charp, 0);
MODULE_PARM_DESC(vaddr_base, "PLC runtime virtual base address of pin controller");

int __init init_module(void) {
	int res;
	long l;
	char* endptr;

	l = simple_strtol(vaddr_base, &endptr, 0);
	if (endptr == NULL) {
		log_err("Unable to cast input address\n");
		return -EINVAL;
	}

	if ( (res = start_io_monitor(p_pid, (void*)l)) )
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
