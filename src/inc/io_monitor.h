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
 *
 * Furthermore, it provides an interface to determine whether a mapping request overlaps
 * with some protected I/O address. This interface is available also if I/O monitor is disabled.
 */

// Map overlap checking interface
#define NOT_OVERLAPPING    	0
#define OVERLAPPING        	1
#define __map_overlaps_io(start, end) do {                              	\
	unsigned i;                                                     	\
	unsigned long b_start, b_end;                                   	\
	for (i = 0; i < PHYS_IO_CONF->blocks; i++) {                    	\
		b_start = (unsigned long)PHYS_IO_CONF->addrs[i];        	\
		b_end = b_start + (unsigned long)PHYS_IO_CONF->sizes[i];	\
		if ( (start >= b_start && start <= b_end) ||            	\
		     (end >= b_start && end <= b_end) )                 	\
			return OVERLAPPING;                             	\
	}                                                               	\
	return NOT_OVERLAPPING;                                         	\
} while(0)


#ifdef IO_MONITOR_ENABLED

#define IO_MONITOR_INTERVAL	10 // ms

int start_io_monitor(int, void*);

void stop_io_monitor(void);

// Map interface delegated to I/O monitor.
int map_overlaps_io(unsigned long start, unsigned long end);

#else

#define start_io_monitor(x,y)	0
#define stop_io_monitor()    	(void)0

// Include only basic I/O configuration to provide map interface.
typedef struct {
	const void** addrs; // Set of address blocks
	const unsigned* sizes; // Size of each block in bytes
	const unsigned blocks; // Number of blocks
	const unsigned size; // Total size
} io_conf_t;
#define PHYS_IO_CONF	((const io_conf_t*)&phys_io_conf)
#include "io_defs.h"

static inline int map_overlaps_io(unsigned long start, unsigned long end) {
	__map_overlaps_io(start, end);
}

#endif

#endif
