#ifndef __BCM2835_H
#define __BCM2835_H

#include <asm/io.h>

/*
 *
 * Broadcom 2835 System-on-Chip used in first generation of Raspberry Pi board.
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/README.md
 *
 * BCM2835 Datasheet: https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 */

/*
 * The target system uses GPIO and I2C for its input and output.
 * To improve the performance, only pin control addresses in use by the system are specified.
 * The unused pins will remain without protection.
 * 
 * WARNING:
 * 	Choose with care the set of pins to be monitored.
 * 	Even though a pin is not actively used,
 * 	multiplexing, misconfiguring or writing into it
 * 	may cause physical damage to the chip as well.
 * 	If the system resources are enough, consider to
 * 	enable the monitor on the entire I/O configuration space.
 * 
 * The target system uses the following I/O pins, having the corresponding control registers:
 *  - Pin  2: multiplexed as I2C SDA		0x20200000
 *  - Pin  3: multiplexed as I2C SCL		0x20200000
 *  - Pin 22: GPIO OUT (LED)        		0x20200008
 *  - Pin 24: GPIO IN (Button)      		0x20200008
 *
 * The protection will be enabled for the above registers,
 * thus preventing the configuration to change during the logic execution.
 *
 * Even though it would not be needed for the target system, the following is a general interface.
 * The target addresses can be specified by means of a certain number of blocks (IO_BLOCKS).
 * Each block is made of base address and size (in bytes).
 * Here only two blocks of 4 bytes are needed (two 32-bit registers).
 */

#define IO_BLOCKS     	2

#define PIN_2_3_BASE  	((void*)0x20200000)
#define PIN_22_24_BASE	((void*)0x20200008)

#define PIN_2_3_SIZE  	4
#define PIN_22_24_SIZE	4

#define TOTAL_SIZE	8


static const void* bcm2835_io_addrs[IO_BLOCKS] = {
	PIN_2_3_BASE,
	PIN_22_24_BASE
};

static const unsigned bcm2835_io_sizes[IO_BLOCKS] = {
	PIN_2_3_SIZE,
	PIN_22_24_SIZE
};

// Fill in the required global struct.
static const io_conf_t phys_io_conf = {
	.addrs = bcm2835_io_addrs,
	.sizes = bcm2835_io_sizes,
	.blocks = IO_BLOCKS,
	.size = TOTAL_SIZE
};


/*
 * Our target registers are all 32-bit wide, we can use u32 for every read/write.
 */

static inline void* read_values(volatile void** addrs, void* values) {
	u32* u32_values = (u32*)values;
	volatile u32** u32_addrs = (volatile u32**)addrs;
	unsigned b, size, offset = 0;

	for (b = 0; b < io_conf->blocks; b++) { // For each block
		// Read block and update offset
		size = io_conf->sizes[b] / sizeof(u32); // Current block size
		ioread32_rep(u32_addrs[b], u32_values + offset, size);
		offset += size;
	}
	
	return values;
}

#ifdef DEBUG

#define WORDS_PER_LINE 4

static inline void dump_values(volatile void** addrs) {
	volatile u32* current_val;
	volatile u32* limit;
	unsigned b, size;

	printk(KERN_INFO "--- Dump of I/O configuration memory ---\n");
	for (b = 0; b < io_conf->blocks; b++) { // For each block
		size = io_conf->sizes[b] / sizeof(u32); // Current block size
		current_val = addrs[b];
		printk(KERN_INFO "Address 0x%08lx (%u bytes):", (long)current_val, io_conf->sizes[b]);
		for (limit = current_val + size; current_val < limit; current_val++) {
			if ((size - (limit - current_val)) % WORDS_PER_LINE == 0) {
				printk(KERN_CONT "\n");
				printk(KERN_INFO "\t");
			}
			printk(KERN_CONT "0x%08x ", ioread32(current_val));
		}
		printk(KERN_CONT "\n");
	}
}

#endif

static inline int check_addrs_in_block(volatile void* block, const void* values, unsigned index) {
	u32* current_val = (u32*)block;
	u32* trusted_val = (u32*)values;
	u32* limit;
	unsigned detect;
	u32 value;
	detect_info_t info;
	info.type = CFG_DETECT;

	for (	limit = current_val + (io_conf->sizes[index] / sizeof(u32)), detect = 0;
		current_val < limit;
		current_val++, trusted_val++	) {
		
		value = ioread32(current_val);
		if (*trusted_val != value) {
			info.target = (void*)current_val;
			info.new_val = value;
			info.old_val = *trusted_val;
			handle_detection(&info);
			detect++;
		}
	}

	return detect;
}

static inline void restore_value(detect_info_t* info) {
	iowrite32(info->old_val, info->target);
}

#endif
