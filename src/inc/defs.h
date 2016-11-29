#ifndef __DEFS_H
#define __DEFS_H

/*
 * Main definitions.
 */

// I/O configuration to monitor, modeled as a set of I/O memory blocks.
typedef struct {
	const void** addrs; // Set of address blocks
	const unsigned* sizes; // Size of each block in bytes
	const unsigned blocks; // Number of blocks
	const unsigned size; // Total size
} io_conf_t;

// Detection types
#define DR_DETECT	0
#define CFG_DETECT	1
#define MAP_DETECT	2

// Information about detected I/O attack
typedef struct detect_info {
	unsigned type;    	// Detection type
	void* target;     	// Target address
	long new_val;     	// I/O value after detection
	long old_val;     	// I/O value before (trusted)
	void* target_info;	// Extra target info (implementation defined)
} detect_info_t;

// Main detection handler
extern void handle_detection(detect_info_t*);

#endif
