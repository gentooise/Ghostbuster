#ifndef __IO_MONITOR_H
#define __IO_MONITOR_H

/*
 * This monitor is responsible for protecting I/O configuration memory from malicious usage.
 * With 'I/O configuration memory' we include at least Pin Multiplexing and Pin Configuration registers,
 * and in general could include all the control registers.
 * In many architectures these registers are the same. The main difference between the two is that
 * pin multiplexing is typically performed only during startup, while pin configuration registers
 * can be modified by the PLC runtime every time a different logic requires a different configuration (e.g. input or output).
 * For other types of register the behaviour depends on the actual use of the register itself,
 * so it is up to the implementation to decide whether it should be protected or not,
 * and whether a particular modification is legitimate or not.
 *
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
 * and eventually restore the trusted state back whenever a change is detected.
 */

#ifdef IO_MONITOR_ENABLED

/*
 * Needs further analysis to find a proper value.
 * It strongly depends on the target system.
 */
#define IO_MONITOR_INTERVAL 10 // ms

int start_io_monitor(int, void*);

void stop_io_monitor(void);

#else

#define start_io_monitor(x,y)	0
#define stop_io_monitor()    	(void)0

#endif

#endif
