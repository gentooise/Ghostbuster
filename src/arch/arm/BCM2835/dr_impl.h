#ifndef __BCM2835_H
#define __BCM2835_H

#include <uapi/linux/hw_breakpoint.h> // For TYPE_INST and TYPE_DATA
#include <asm/hw_breakpoint.h>
#include <linux/kallsyms.h>

#include <asm/patch.h>

/*
 * Broadcom 2835 System-on-Chip used in the first generation of Raspberry Pi board.
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/README.md
 *
 * BCM2835 Datasheet: https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 *
 * BCM2835 is based on an ARMv6 architecture, containing an ARM1176JZF-S processor.
 * ARM1176JZF-S Technical Reference Manual: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0301h/DDI0301H_arm1176jzfs_r0p7_trm.pdf
 */


/********************** Kernel side DR protection **********************/

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
int (*get_dr_slots)(int) = NULL;
static inline unsigned count_drs(void) {
	if (!get_dr_slots) {
		get_dr_slots = (int (*)(int))kallsyms_lookup_name("hw_breakpoint_slots");
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

static inline void __restore_dr_state(dr_detect_t* info) {
	u32* u32_state = (u32*)info->old_state;

	if (info->index < bp_slots) { // Breakpoint
		WRITE_WB_REG(ARM_OP2_BVR, info->index, *u32_state); // Write breakpoint value register
		WRITE_WB_REG(ARM_OP2_BCR, info->index, *(u32_state+1)); // Write breakpoint control register
	} else { // Watchpoint
		WRITE_WB_REG(ARM_OP2_WVR, info->index - bp_slots, *u32_state); // Write watchpoint value register
		WRITE_WB_REG(ARM_OP2_WCR, info->index - bp_slots, *(u32_state+1)); // Write watchpoint control register
	}
}


/*********************** User side DR protection ***********************/

/*
 * In order to disable DR user interface, we overwrite the prologue of the target functions.
 * The prologue is replaced with the following:
 *  1. Move our return value into R0 (if function is non-void): "mov r0, #-13" -> "mvn r0, #0xC".
 *     We return -EACCES (-13), defined in <uapi/asm-generic/errno-base.h> as "Permission denied".
 *  2. Branch to link register: "bx lr".
 * 
 * The ARM code for the above instructions is the following (http://armconverter.com/):
 * u32 opcodes[2] = {
 * 	0xE3E0000C, // Opcode "0C00E0E3" little-endian
 * 	0xE12FFF1E  // Opcode "1EFF2FE1" little-endian
 * }
 * The thumb version is not supported here.
 * 
 * The functions which need to be disabled are the following (from "linux/hw_breakpoint.h"):
 * 
 * - static struct perf_event* register_user_hw_breakpoint(struct perf_event_attr* attr,
 *                                                         perf_overflow_handler_t triggered,
 * 							struct task_struct* tsk);
 * - static int modify_user_hw_breakpoint(struct perf_event *bp, struct perf_event_attr *attr);
 * - static void unregister_hw_breakpoint(struct perf_event *bp);
 * 
 * See the code below for details.
 * (Assembly code from kernel objdump, source code in "kernel/hw_breakpoint.c").
 * 
 * 1. struct perf_event* register_user_hw_breakpoint(...) ------------------------
 * 
 * c00d3cb0 <register_user_hw_breakpoint>:                                 (Patch)
 * c00d3cb0: e1a0c00d  mov  ip, sp                    ‾|              ‾|<- mvn r0, #0xC
 * c00d3cb4: e92dd800  push {fp, ip, lr, pc}           |<- Prologue   _|<- bx lr
 * c00d3cb8: e24cb004  sub  fp, ip, #4                 |
 * c00d3cbc: e24dd008  sub  sp, sp, #8                _|
 * 
 * c00d3cc0: e92d4000  push {lr}                      ‾|<- Ftrace profiling
 * c00d3cc4: ebfd0302  bl   c00148d4 <__gnu_mcou...>  _|
 * 
 * c00d3cc8: e1a0c001  mov  ip, r1                    ‾|
 * c00d3ccc: e58d2000  str  r2, [sp]                   |
 * c00d3cd0: e3e01000  mvn  r1, #0                     |<- Body
 * c00d3cd4: e1a02003  mov  r2, r3                     |
 * c00d3cd8: e1a0300c  mov  r3, ip                     |
 * c00d3cdc: ebffe3f4  bl   c00cccb4 <perf_event...>  _|
 * 
 * c00d3ce0: e24bd00c  sub  sp, fp, #12               ‾|<- Epilogue
 * c00d3ce4: e89da800  ldm  sp, {fp, sp, pc}          _|
 * 
 * -------------------------------------------------------------------------------
 *
 *
 * 2. int modify_user_hw_breakpoint(...) -----------------------------------------
 * 
 * c00d3e30 <modify_user_hw_breakpoint>:                                                      (Patch)
 * c00d3e30: e1a0c00d  mov  ip, sp                                       ‾|              ‾|<- mvn r0, #0xC
 * c00d3e34: e92ddff0  push {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}  |<- Prologue   _|<- bx lr
 * c00d3e38: e24cb004  sub  fp, ip, #4                                    |
 * c00d3e3c: e24dd00c  sub  sp, sp, #12                                  _|
 * 
 * c00d3e40: e92d4000  push {lr}                                         ‾|<- Ftrace profiling
 * c00d3e44: ebfd02a2  bl   c00148d4 <__gnu_mcount_nc>                   _|
 * 
 * c00d3e48: e1a0a000  mov  sl, r0                                       ‾|
 * ...       ...       ...  ...                                           |<- Body (1)
 * c00d3ed8: e24bd028  sub  sp, fp, #40                                  _|
 *                                                                       _
 * c00d3edc: e89daff0  ldm  sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}  |<- Epilogue
 *                                                                       ‾
 * c00d3ee0: e1a0000a  mov  r0, sl                                       ‾|
 * ...       ...       ...  ...                                           |<- Body (2)
 * c00d3f30: eaffffd8  b    c00d3e98 <modify_user_hw_breakpoint+0x68>    _|
 * 
 * -------------------------------------------------------------------------------
 *
 *
 * 3. void unregister_hw_breakpoint(...) -----------------------------------------
 * 
 * c00d3ce8 <unregister_hw_breakpoint>:                               _    (Patch)
 * c00d3ce8: e1a0c00d  mov   ip, sp                   ‾|               |<- bx lr
 * c00d3cec: e92dd800  push  {fp, ip, lr, pc}          |<- Prologue   ‾
 * c00d3cf0: e24cb004  sub   fp, ip, #4               _|
 * 
 * c00d3cf4: e92d4000  push  {lr}                     ‾|<- Ftrace profiling
 * c00d3cf8: ebfd02f5  bl    c00148d4 <__gnu_mcou...> _|
 * 
 * c00d3cfc: e3500000  cmp   r0, #0                   ‾|
 * c00d3d00: 089da800  ldmeq sp, {fp, sp, pc}          |<- Body
 * c00d3d04: ebffe4fa  bl    c00cd0f4 <perf_event...> _|
 *                                                    _
 * c00d3d08: e89da800  ldm   sp, {fp, sp, pc}          |<- Epilogue
 *                                                    ‾
 * -------------------------------------------------------------------------------
 */

#if IS_ENABLED(CONFIG_THUMB2_KERNEL)

#error Thumb mode not supported

#else

// ARM mode
#define __INSTRUCTION_SIZE     	4

#define __REGISTER_USER_DR_SIZE	(2 * __INSTRUCTION_SIZE)
#define __MODIFY_USER_DR_SIZE  	(2 * __INSTRUCTION_SIZE)
#define __UNREGISTER_DR_SIZE   	(1 * __INSTRUCTION_SIZE)

u32 __register_user_dr_new[__REGISTER_USER_DR_SIZE / sizeof(u32)] = {
	0xE3E0000C, // Opcode "0C00E0E3" little-endian for "mvn r0, #0xC"
	0xE12FFF1E  // Opcode "1EFF2FE1" little-endian for "bx lr"
};

u32 __modify_user_dr_new[__MODIFY_USER_DR_SIZE / sizeof(u32)] = {
	0xE3E0000C, // Opcode "0C00E0E3" little-endian for "mvn r0, #0xC"
	0xE12FFF1E  // Opcode "1EFF2FE1" little-endian for "bx lr"
};

u32 __unregister_dr_new[__UNREGISTER_DR_SIZE / sizeof(u32)] = {
	0xE12FFF1E  // Opcode "1EFF2FE1" little-endian for "bx lr"
};

static void (*_patch_text)(void*, unsigned) = NULL;
static inline void patch_ktext(void* addr, void* new_text, void* old_text, unsigned size) {
	u32* u32_addr = (u32*)addr;
	u32* u32_old = (u32*)old_text;
	u32* u32_new = (u32*)new_text;
	unsigned i;
	if (!_patch_text) {
		_patch_text = (void (*)(void*, unsigned))kallsyms_lookup_name("patch_text");
	}
	// patch_text() calls stop_machine() and flushes the text page for each instruction inserted.
	// This could be optimized by using stop_machine() here to wrap the entire patch,
	// and by calling __patch_text_real() instead of patch_text().
	// Furthermore, instead of __patch_text_real(), patching could be done manually here,
	// refreshing the kernel page only at the end.
	// However, since this is only executed at module startup and shutdown,
	// such an optimization is not really needed, and no wish to reinvent the wheel.
	for (i = 0; i < size / sizeof(u32); i++) {
		*(u32_old + i) = *(u32_addr + i); // Save 
		_patch_text(u32_addr + i, *(u32_new + i)); // From asm/patch.h
	}
}

#endif

#endif
