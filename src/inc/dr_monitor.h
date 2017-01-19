#ifndef __DR_MONITOR_H
#define __DR_MONITOR_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

/*
 * This monitor is responsible for protecting debug registers from malicious usage.
 * There are basically two ways to use debug registers:
 *  1. Kernel space
 *  2. User space
 * 
 * From kernel space, it is possible to modify debug registers by means of one of the following:
 *  - "register_wide_hw_breakpoint" kernel interface to register breakpoints or watchpoints;
 *  - direct debug register access through CPU instructions (depending on the architecture).
 * 
 * From user space, direct access to debug registers is forbidden in Linux,
 * so the only way to use them is through the ptrace syscall interface (or similar).
 * When a user process requests an hardware breakpoint, ptrace uses the following function:
 *  - "register_user_hw_breakpoint"
 * Upon request, the hardware breakpoint is bound to the task, so that the kernel automatically
 * writes the corresponding debug registers when the task is scheduled, and it resets them back
 * when the process is switched out from the CPU.
 * 
 * To protect debug registers from kernel space attack, this monitor should check their state
 * every DR_MONITOR_INTERVAL milliseconds, and eventually restore their trusted state back whenever a change is detected.
 * However, the kernel side check is not enough, because when this module is running
 * it is only capable of reading the kernel debug register, while the user space debug
 * registers are stored inside the corresponding task_struct of the process and will be available only
 * during execution of that process. Thus, to protect debug registers from user space access as well,
 * this monitor should also modify the behaviour of register_user_hw_breakpoint.
 * Since this debug register user interface is often used for debug purposes only,
 * and it is not needed on a deployed system, this monitor may simply disable it.
 *
 * Furthermore, even if this monitor is disabled, an interface to have access to debug registers is always available
 * for other parts of the module (e.g. I/O monitor needs it to intercept read/write operations of the PLC logic).
 * When the DR monitor is enabled, it mediates the access to DRs, so that they can be used only through this interface.
 */

typedef void (*dr_handler_t)(struct perf_event*, struct perf_sample_data*, struct pt_regs*);

static inline void* __set_dr(int pid, void* vaddr, dr_handler_t handler, unsigned type) {
	struct perf_event_attr attr;
	hw_breakpoint_init(&attr);
	attr.bp_addr = (unsigned long)vaddr;
	attr.bp_len = HW_BREAKPOINT_LEN_4;
	attr.bp_type = type;
	/*if (pid > 0) { // Not supported for now, requires re-enabling user side interface
		tsk = pid_task(find_vpid(ppid), PIDTYPE_PID);
		if (!tsk) return NULL;
		return (void*)register_user_hw_breakpoint(&attr, handler, NULL, tsk);
	}*/
	return register_wide_hw_breakpoint(&attr, handler, NULL);
}

static inline void __reset_dr(void* dr) {
	if (dr) {
		unregister_wide_hw_breakpoint((struct perf_event**)dr);
	}
}

#ifdef DR_MONITOR_ENABLED

/*
 * Needs further analysis to find a proper value.
 * It strongly depends on the target system.
 */
#define DR_MONITOR_INTERVAL 50 // ms

int start_dr_monitor(void);

void stop_dr_monitor(void);

// Debug registers interface
void* set_read_dr(int pid, void* vaddr, dr_handler_t handler);
void* set_write_dr(int pid, void* vaddr, dr_handler_t handler);
void reset_dr(void*);

#else

#define start_dr_monitor()   	0
#define stop_dr_monitor()    	(void)0

#define set_read_dr(p, v, h) 	__set_dr(p, v, h, HW_BREAKPOINT_R)
#define set_write_dr(p, v, h)	__set_dr(p, v, h, HW_BREAKPOINT_W)
#define reset_dr(d)          	__reset_dr(d)

#endif

#endif
