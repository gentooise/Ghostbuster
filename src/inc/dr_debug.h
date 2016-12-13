#ifndef __DR_DEBUG_H
#define __DR_DEBUG_H

#include "log.h"
#include "dr_conf.h"

/*
 * DR debug subsystem.
 *
 * In order to simplify debug, this file provides macros to dump
 * the state of the resources monitored by DR monitor.
 *
 * Since the number of DR registers is not known at compile time,
 * and to avoid the use of dynamic memory with kmalloc (which may fail),
 * here the maximum number of DR registers is supposed to be 32
 * (which is quite large for almost every architecture).
 * If it's not enough, modify it accordingly to the specific implementation.
 */

#ifdef DR_DEBUG // DR debug subsystem enabled

#define DR_MAX_STATE_SIZE	(DR_STATE_SIZE * 32)

char dr_state[DR_MAX_STATE_SIZE];
static void __dump_dr_state(volatile void* state) {
	unsigned i, j;
	for (i = 0; i < count_drs(); i++) {
		log_info("\t#%u\t", i);
		for (j = 0; j < DR_STATE_SIZE; j++) {
			log_cont("%02x ", *(char*)(state+j));
		}
		state += DR_STATE_SIZE;
		log_cont("\n");
	}
}
static void __dump_dr_current_state(void) {
	get_dr_state((void*)dr_state);
	__dump_dr_state(dr_state);
}

#define dump_dr_state() do {                  	\
	log_info("--- Start DR dump ---\n");  	\
	log_info("DR trusted state:\n");      	\
	__dump_dr_state((void*)trusted_state);	\
	log_info("DR current state:\n");      	\
 	__dump_dr_current_state();            	\
	log_info("--- End DR dump ---\n");    	\
} while (0)

#else // DR debug subsystem disabled

#define dump_dr_state() 	(void)0

#endif

#endif
