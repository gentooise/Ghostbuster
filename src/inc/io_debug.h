#ifndef __IO_DEBUG_H
#define __IO_DEBUG_H

#include "log.h"
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
	for (b = 0, offset = 0; b < PHYS_IO_CONF->blocks; b++) { // For each block
		size = PHYS_IO_CONF->sizes[b]; // Current block size
		log_info("\tAddress 0x%08lx (%u bytes):", (long)PHYS_IO_CONF->addrs[b], size);
		for (i = 0; i < size; i++) {
			if (i % MAX_BYTES_PER_LINE == 0) {
				log_cont("\n");
				log_info("\t\t");
			}
			log_cont("%02x ", *(char*)(state+i+offset));
		}
		offset += size;
		log_cont("\n");
	}
}
static void __dump_io_current_state(volatile void** addrs) {
	get_io_state(addrs, (void*)io_state);
	__dump_io_state(io_state);
}

#define dump_io_state() do {                  	\
	log_info("--- Start I/O dump ---\n"); 	\
	log_info("I/O trusted state:\n");     	\
	__dump_io_state((void*)trusted_state);	\
	log_info("I/O current state:\n");     	\
	__dump_io_current_state(addrs);       	\
	log_info("--- End I/O dump ---\n");   	\
} while (0)

#else // I/O debug subsystem disabled

#define dump_io_state()   	(void)0

#endif

#endif
