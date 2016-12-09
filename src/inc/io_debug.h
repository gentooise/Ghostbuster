#ifndef __IO_DEBUG_H
#define __IO_DEBUG_H

#include <io_conf.h>

/*
 * I/O debug subsystem.
 *
 * In order to simplify debug, this file provides macros to dump
 * the state of the resources monitored by I/O monitor.
 */

#ifdef IO_DEBUG // I/O debug subsystem enabled

#define MAX_BYTES_PER_LINE	16

char io_state[IO_STATE_TOTAL_SIZE];
static void __dump_io_state(volatile void* state) {
	unsigned i, b, size, offset;
	for (b = 0, offset = 0; b < io_conf->blocks; b++) { // For each block
		size = io_conf->sizes[b]; // Current block size
		printk(KERN_INFO "\tAddress 0x%08lx (%u bytes):", (long)io_conf->addrs[b], size);
		for (i = 0; i < size; i++) {
			if (i % MAX_BYTES_PER_LINE == 0) {
				printk(KERN_CONT "\n");
				printk(KERN_INFO "\t\t");
			}
			printk(KERN_CONT "%02x ", *(char*)(state+i+offset));
		}
		offset += size;
		printk(KERN_CONT "\n");
	}
}
static void __dump_io_current_state(volatile void** addrs) {
	get_io_state(addrs, (void*)io_state);
	__dump_io_state(io_state);
}

#define dump_io_state() do {                         	\
	printk(KERN_INFO "--- Start I/O dump ---\n");	\
	printk(KERN_INFO "I/O trusted state:\n");    	\
	__dump_io_state((void*)trusted_state);       	\
	printk(KERN_INFO "I/O current state:\n");    	\
	__dump_io_current_state(addrs);              	\
	printk(KERN_INFO "--- End I/O dump ---\n");  	\
} while (0)

#else // I/O debug subsystem disabled

#define dump_io_state()   	(void)0

#endif

#endif
