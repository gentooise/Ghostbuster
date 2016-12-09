#ifndef __BCM2835_H
#define __BCM2835_H

#include <uapi/linux/hw_breakpoint.h> // For TYPE_INST and TYPE_DATA
#include <asm/hw_breakpoint.h>
#include <linux/kallsyms.h>

/*
 *
 * Broadcom 2835 System-on-Chip used in the first generation of Raspberry Pi board.
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/README.md
 *
 * BCM2835 Datasheet: https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 * BCM2835 is based on an ARMv6 architecture, containing an ARM1176JZF-S processor.
 * ARM1176JZF-S Technical Reference Manual: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0301h/DDI0301H_arm1176jzfs_r0p7_trm.pdf
 */

/*
 * On the target processor, each DR is made of a pair of value and control registers.
 * Each register is 32-bit wide, so each DR state needs 8 bytes of data.
 */
#define __DR_U32_STATE_SIZE	2
#define __DR_STATE_SIZE		(__DR_U32_STATE_SIZE * sizeof(u32)) // 8 bytes

/*
 * Macros to read/write to/from watchpoint registers
 * See also: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0301h/Cegjbjed.html
 *
 * Derived from macros in <asm/hw_breakpoint.h> and <arch/arm/kernel/hw_breakpoint.c>:
 *
 * 	#define ARM_DBG_READ(N, M, OP2, VAL) do {\
 * 		asm volatile("mrc p14, 0, %0, " #N "," #M ", " #OP2 : "=r" (VAL));\
 * 	} while (0)
 *
 * 	#define ARM_DBG_WRITE(N, M, OP2, VAL) do {\
 * 		asm volatile("mcr p14, 0, %0, " #N "," #M ", " #OP2 : : "r" (VAL));\
 * 	} while (0)
 * 
 * - N is 0 (c0)
 * - M is number from 0 to 15 (watchpoint index) (cy)
 * - OP2 is Opcode_2
 *     #define ARM_OP2_WVR 6 // Opcode_2 for value register
 *     #define ARM_OP2_WCR 7 // Opcode_2 for control register
 * - VAL is the value to read/write
 *
 * We need switch-cases to hard-code the index number (M) into the instruction (cy).
 */

#define READ_WB_REG_CASE(OP2, M, VAL)                   \
	case (M):                                       \
		ARM_DBG_READ(c0, c ## M, OP2, VAL);     \
		break

#define WRITE_WB_REG_CASE(OP2, M, VAL)                  \
	case (M):                                       \
		ARM_DBG_WRITE(c0, c ## M, OP2, VAL);    \
		break

#define GEN_READ_WB_REG_CASES(OP2, VAL)   \
	READ_WB_REG_CASE(OP2, 0, VAL);    \
	READ_WB_REG_CASE(OP2, 1, VAL);    \
	READ_WB_REG_CASE(OP2, 2, VAL);    \
	READ_WB_REG_CASE(OP2, 3, VAL);    \
	READ_WB_REG_CASE(OP2, 4, VAL);    \
	READ_WB_REG_CASE(OP2, 5, VAL);    \
	READ_WB_REG_CASE(OP2, 6, VAL);    \
	READ_WB_REG_CASE(OP2, 7, VAL);    \
	READ_WB_REG_CASE(OP2, 8, VAL);    \
	READ_WB_REG_CASE(OP2, 9, VAL);    \
	READ_WB_REG_CASE(OP2, 10, VAL);   \
	READ_WB_REG_CASE(OP2, 11, VAL);   \
	READ_WB_REG_CASE(OP2, 12, VAL);   \
	READ_WB_REG_CASE(OP2, 13, VAL);   \
	READ_WB_REG_CASE(OP2, 14, VAL);   \
	READ_WB_REG_CASE(OP2, 15, VAL)

#define GEN_WRITE_WB_REG_CASES(OP2, VAL)  \
	WRITE_WB_REG_CASE(OP2, 0, VAL);   \
	WRITE_WB_REG_CASE(OP2, 1, VAL);   \
	WRITE_WB_REG_CASE(OP2, 2, VAL);   \
	WRITE_WB_REG_CASE(OP2, 3, VAL);   \
	WRITE_WB_REG_CASE(OP2, 4, VAL);   \
	WRITE_WB_REG_CASE(OP2, 5, VAL);   \
	WRITE_WB_REG_CASE(OP2, 6, VAL);   \
	WRITE_WB_REG_CASE(OP2, 7, VAL);   \
	WRITE_WB_REG_CASE(OP2, 8, VAL);   \
	WRITE_WB_REG_CASE(OP2, 9, VAL);   \
	WRITE_WB_REG_CASE(OP2, 10, VAL);  \
	WRITE_WB_REG_CASE(OP2, 11, VAL);  \
	WRITE_WB_REG_CASE(OP2, 12, VAL);  \
	WRITE_WB_REG_CASE(OP2, 13, VAL);  \
	WRITE_WB_REG_CASE(OP2, 14, VAL);  \
	WRITE_WB_REG_CASE(OP2, 15, VAL)

#define READ_WB_REG(OP2, I, VAL)                                                  \
	switch (I) {                                                              \
		GEN_READ_WB_REG_CASES(OP2, VAL);                                  \
	default:                                                                  \
		printk(KERN_ERR "Debug register index out of range: %u\n", I);    \
	}

#define WRITE_WB_REG(OP2, I, VAL)                                                 \
	switch (I) {                                                              \
		GEN_WRITE_WB_REG_CASES(OP2, VAL);                                 \
	default:                                                                  \
		printk(KERN_ERR "Debug register index out of range: %u\n", I);    \
	}

// Debug register slots
static unsigned bp_slots, wp_slots, dr_slots;

// This call is also architecture-dependent, there is no interface to provide
// the number of available DRs in linux/hw_breakpoint.h.
int (*get_dr_slots)(int);
static inline unsigned count_drs(void) {
	if (!get_dr_slots) {
		get_dr_slots = (void*) kallsyms_lookup_name("hw_breakpoint_slots");
		// TYPE_INST refers to breakpoints, TYPE_DATA refers to watchpoints
		bp_slots = get_dr_slots(TYPE_INST);
		wp_slots = get_dr_slots(TYPE_DATA);
		dr_slots = bp_slots + wp_slots;
	}
	return dr_slots;
}

static inline void get_dr_state(void* state) {
	unsigned i;
	u32* u32_state = (u32*)state;

	for (i = 0; i < bp_slots;  i++) {
		READ_WB_REG(ARM_OP2_BVR, i, *u32_state); // Read breakpoint value register
		READ_WB_REG(ARM_OP2_BCR, i, *(u32_state+1)); // Read breakpoint control register
		u32_state += __DR_U32_STATE_SIZE;
	}
	for (i = 0; i < wp_slots; i++) {
		READ_WB_REG(ARM_OP2_WVR, i, *u32_state); // Read watchpoint value register
		READ_WB_REG(ARM_OP2_WCR, i, *(u32_state+1)); // Read watchpoint control register
		u32_state += __DR_U32_STATE_SIZE;
	}
}

// Static detection information: detections are handled sequentially.
u32 new_state_buf[__DR_U32_STATE_SIZE];
u32 old_state_buf[__DR_U32_STATE_SIZE];
static dr_detect_t info = {
	.new_state = (void*)new_state_buf,
	.old_state = (void*)old_state_buf,
	.index = 0 // Given at detection time
};
static inline void check_dr_state(const void* trusted_state) {
	unsigned i;
	u32 value = 0, cntrl = 0;
	const u32* u32_t_state = (u32*)trusted_state;

	for (i = 0; i < bp_slots;  i++) {
		READ_WB_REG(ARM_OP2_BVR, i, value); // Read breakpoint value register
		READ_WB_REG(ARM_OP2_BCR, i, cntrl); // Read breakpoint control register
		if (value != *u32_t_state || cntrl != *(u32_t_state+1)) {
			*(u32*)(info.new_state) = value;
			*(u32*)(info.new_state + sizeof(u32)) = cntrl;
			info.old_state = (void*)u32_t_state;
			info.index = i;
			handle_dr_detection(&info);
		}
		u32_t_state += __DR_U32_STATE_SIZE;
	}
	for (i = 0; i < wp_slots; i++) {
		READ_WB_REG(ARM_OP2_WVR, i, value); // Read watchpoint value register
		READ_WB_REG(ARM_OP2_WCR, i, cntrl); // Read watchpoint control register
		if (value != *u32_t_state || cntrl != *(u32_t_state+1)) {
			*(u32*)(info.new_state) = value;
			*(u32*)(info.new_state + sizeof(u32)) = cntrl;
			info.old_state = (void*)u32_t_state;
			info.index = i + bp_slots;
			handle_dr_detection(&info);
		}
		u32_t_state += __DR_U32_STATE_SIZE;
	}
}

static inline void restore_dr_state(dr_detect_t* info) {
	u32* u32_state = (u32*)info->old_state;

	if (info->index < bp_slots) { // Breakpoint
		WRITE_WB_REG(ARM_OP2_BVR, info->index, *u32_state); // Write breakpoint value register
		WRITE_WB_REG(ARM_OP2_BCR, info->index, *(u32_state+1)); // Write breakpoint control register
	} else { // Watchpoint
		WRITE_WB_REG(ARM_OP2_WVR, info->index - bp_slots, *u32_state); // Write watchpoint value register
		WRITE_WB_REG(ARM_OP2_WCR, info->index - bp_slots, *(u32_state+1)); // Write watchpoint control register
	}
}

#endif
