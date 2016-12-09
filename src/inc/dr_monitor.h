#ifndef __DR_MONITOR_H
#define __DR_MONITOR_H

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
 */

#ifdef DR_MONITOR_ENABLED

/*
 * Needs further analysis to find a proper value.
 * It strongly depends on the target system.
 */
#define DR_MONITOR_INTERVAL 50 // ms

int start_dr_monitor(void);

void stop_dr_monitor(void);

#else

#define start_dr_monitor()	0
#define stop_dr_monitor() 	(void)0

#endif

#endif
