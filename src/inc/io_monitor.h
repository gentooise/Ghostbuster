#ifndef __IO_MONITOR_H
#define __IO_MONITOR_H

/*
 * This monitor is responsible for protecting I/O configuration memory from malicious usage.
 * I/O memory can be accessed from:
 *  1. Kernel space
 *  2. User space
 * 
 * Since I/O memory is seen as a set of physical addresses, and the system is running with an active MMU,
 * the only way to access I/O is to map physical addresses to virtual addresses.
 *
 * From kernel space, the mapping can be done via "ioremap", or manually (doing by hand what ioremap does).
 * From user space, the mapping can be achieved by mmap() and the special device /dev/mem.
 *
 * After mapping a target physical address, both kernel and user processes can use their own _DIFFERENT_ virtual addresses
 * to read/write the _SAME_ physical I/O memory. So, if I/O memory is written by a kernel (or a user) program,
 * the effects are visible from any other process (kernel or user) which has the same physical address mapped.
 * From this point of view, I/O memory is a kind of "shared memory" available to anyone who has asked for a mapping.
 * Thus, to protect I/O memory from any attack (kernel or user), it is enough to monitor its state from kernel space.
 * This monitor should check the target I/O memory every DR_MONITOR_INTERVAL milliseconds,
 * and eventually restore the trusted state back whenever a change is detected
 */

#ifdef IO_MONITOR_ENABLED

/*
 * Needs further analysis to find a proper value.
 * It strongly depends on the target system.
 */
#define IO_MONITOR_INTERVAL 50 // ms

int start_io_monitor(void);

void stop_io_monitor(void);

#else

#define start_io_monitor()	0
#define stop_io_monitor() 	(void)0

#endif

#endif
