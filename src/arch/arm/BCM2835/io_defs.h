#ifndef __IO_DEFS_H
#define __IO_DEFS_H

/*
 * BCM2835 I/O Configuration registers.
 *
 * Broadcom 2835 System-on-Chip used in the first generation of Raspberry Pi board.
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/README.md
 *
 * BCM2835 Datasheet: https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 */

/*
 * We want to protect 6 different I/O control registers in the target system, starting from physical address 0x20200000,
 * for both pin multiplexing and pin configuration. Each register is 32-bit wide, having
 * 3 bits for each I/O pin, for a total of 10 pins managed by each register.
 */
#define IO_BLOCKS            	1 // Registers are contiguous, only one block needed
#define PINS_PER_REG         	10

// Block 1
#define PIN_CTRL_BASE        	((void*)0x20200000) // Pin controller start address
#define PIN_CTRL_SIZE        	24 // 6 regs * 4 bytes each

// Other blocks here...
// #define ...

#define __IO_STATE_TOTAL_SIZE	24 // Total size of I/O memory to monitor in bytes

/*
 * Pin modes:       _
 *   000 Input       | Pin Configuration
 *   001 Output     _|
 *   010 Function 0  |
 *   ... ...         | Pin Multiplexing
 *   111 Function 5 _|
 */
#define CTRL_BITS_PER_PIN    	3
#define __PIN_CTRL_MASK        	0x07 // 111b: the three control bits of a pin
#define __PIN_MUX_MASK         	0x06 // 110b: the most significant bits are for multiplexing 
#define __PIN_CONF_MASK        	0x01 // 001b: the least significant bit is for configuration (input/output)

#define PIN_CTRL_MASK(p)     	(__PIN_CTRL_MASK << (p * CTRL_BITS_PER_PIN))
#define PIN_MUX_MASK(p)      	(__PIN_MUX_MASK << (p * CTRL_BITS_PER_PIN))
#define PIN_CONF_MASK(p)     	(__PIN_CONF_MASK << (p * CTRL_BITS_PER_PIN))

/*
 * Registers needed for checking the actual operation of PLC runtime.
 * We have 3 types of registers in BCM2835:
 *   - Level registers: used to READ the value of a pin
 *   - Clear registers: used to WRITE 0 to the value of a pin
 *   - Set registers: used to WRITE 1 to the value of a pin
 *
 * For each type, we have 2 registers: the first one is for pins [0-31],
 * the second one is for pins [32-53] (some bits unused).
 *
 * Since we want to monitor how these register are used by the PLC logic,
 * we need relative offsets with respect to physical address 0x20200000.
 * These offsets will be added to the virtual starting address mapped
 * by the PLC runtime, and the result will be provided to debug registers.
 */
#define REG_NUM     	2

#define REG_LEV0    	0x34
#define REG_LEV1    	0x38

#define REG_CLR0    	0x28
#define REG_CLR1    	0x2C

#define REG_SET0    	0x1C
#define REG_SET1    	0x20

static const unsigned lev_regs[REG_NUM] = {
	REG_LEV0,
	REG_LEV1
};

static const unsigned clr_regs[REG_NUM] = {
	REG_CLR0,
	REG_CLR1
};

static const unsigned set_regs[REG_NUM] = {
	REG_SET0,
	REG_SET1
};

// Macros for global pin number and LEV, CLR, SET registers
// Global pin range: [0-63] -> 000000 - 111111
// Bit 5 -----> index
// Bit [0-4] -> shift
#define PIN_INDEX(p)	(((p) & 32) >> 5) // bit 5
#define PIN_SHIFT(p)	((p) & 31) // bits 0-4

#define LEV_REG(pin)	(lev_regs[PIN_INDEX(pin)])
#define CLR_REG(pin)	(clr_regs[PIN_INDEX(pin)])
#define SET_REG(pin)	(set_regs[PIN_INDEX(pin)])


static const void* bcm2835_io_addrs[IO_BLOCKS] = {
	PIN_CTRL_BASE
};

static const unsigned bcm2835_io_sizes[IO_BLOCKS] = {
	PIN_CTRL_SIZE
};

// Fill in the required global struct.
static const io_conf_t phys_io_conf = {
	.addrs = bcm2835_io_addrs,
	.sizes = bcm2835_io_sizes,
	.blocks = IO_BLOCKS,
	.size = __IO_STATE_TOTAL_SIZE
};


#define IO_BLOCK_SIZE(b) (phys_io_conf.sizes[b] / sizeof(u32))

#endif
