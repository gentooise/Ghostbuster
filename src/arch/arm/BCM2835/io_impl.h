#ifndef __IO_IMPL_H
#define __IO_IMPL_H

#include <asm/io.h>
#include <linux/delay.h>

#include "io_defs.h"
#include "dr_monitor.h"

/*
 * We monitor pin configuration and pin multiplexing registers (which are the same registers in BCM2835).
 * Here, for simplicity, other configuration registers are ignored, but they could be protected as well
 * by defining an expected behaviour and detecting changes that do not follow this behaviour.
 * We do this only for pin multiplexing and pin configuration registers, as follows.
 *
 * For pin multiplexing, the monitor verifies that the I/O configuration never changes. If such a change occurs, then the
 * monitor consider this case as a Pin Control Attack attempt. This is a fair assumption, because it supposes that the physical
 * connections of our system are decided before it is powered on, and cannot change during run-time.
 *
 * For pin configuration, it is possible (and better) to do more. When a pin configuration change occurs, it could come either
 * from Pin Control Attack or a legitimate PLC runtime operation. To automatically distinguish between the two cases, the I/O monitor
 * does the following: given a mapping between I/O configuration registers and the corresponding write (or clear) registers,
 * it checks if the operations currently performed on a pin are in line with its actual configuration after a change.
 * For instance, if a pin has just been configured as output pin, any subsequent operation made by the PLC logic on that pin
 * must be a _WRITE_ operation (or vice versa: input -> read). If it is not the case, probably we are under Pin Control Attack.
 * We do not make any assumption on the actual pins used by the logic as input or output. Thus, the PLC runtime can modify the
 * Pin Configuration registers according to the uploaded logic. We leverage debug registers to check that the new configuration
 * is in line with the operations performed by the PLC logic.
 *
 * In conclusion, we detect Pin Control Attack in the following cases:
 *   1) A Pin Multiplexing register has been changed
 *   2) A Pin Configuration register has been changed, and it is not conforming with actual operation performed by the PLC logic.
 */

/*
 * Our target platform has all 32-bit wide registers, we can use u32 for every read/write.
 */

static inline void get_io_state(volatile void** addrs, void* state) {
	u32* u32_state = (u32*)state;
	volatile u32** u32_addrs = (volatile u32**)addrs;
	unsigned b, w;

	for (b = 0; b < phys_io_conf.blocks; b++) { // For each block
		for (w = 0; w < IO_BLOCK_SIZE(b); w++) { // For each word in block
			*(u32_state++) = ioread32(u32_addrs[b] + w);
		}
	}
}

typedef struct {
	unsigned pin; // Global pin number
	unsigned reg_pin; // Pin offset into control register
	u32 diff;
	u32* trusted;
} target_info_t;
static io_detect_t info; // Static detection information: detections are handled sequentially.
static target_info_t tinfo;
static inline void check_io_state(volatile void* block, const void* state, unsigned index) {
	u32* current_val = (u32*)block;
	u32* trusted_val = (u32*)state;
	u32* limit;
	u32 value;
	unsigned reg_pin, pin = 0;
	u32 diff;

	for (	limit = current_val + IO_BLOCK_SIZE(index);
		current_val < limit;
		current_val++, trusted_val++	) { // For each register
		
		value = ioread32(current_val);
		for (reg_pin = 0; reg_pin < PINS_PER_REG; reg_pin++, pin++) { // For each pin in register
			diff = (value ^ (*trusted_val)) & PIN_CTRL_MASK(reg_pin);
			if (diff) {
				info.target = (void*)current_val;
				info.new_val = value;
				info.old_val = *trusted_val;
				tinfo.pin = pin;
				tinfo.reg_pin = reg_pin;
				tinfo.diff = diff;
				tinfo.trusted = trusted_val;
				info.target_info = (void*)&tinfo;
				handle_io_detection(&info);
			}
		}
	}
}


/*
 * All the information about PLC logic operations have been found by directly
 * inspecting PLC logic code from watchpoints through the address pointed by WFAR register.
 *
 *   u32* instr;
 *   asm volatile("mrc p14, 0, %0, c0, c6, 0" : "=r" (instr)); // Read WFAR (Watchpoint Fault Address Register)
 *   instr -= 2; // WFAR contains the address of the instruction that generated the watchpoint, plus 0x8.
 *   log_info("Instruction: address = 0x%08x, opcode = 0x%08x\n", (u32)instr, *instr);
 *
 */

static void* volatile hw_break = NULL;
static DEFINE_MUTEX(dr_lock);
#define atomic_reset_dr(dr) do {	\
	mutex_lock(&dr_lock);   	\
	if (dr) reset_dr(dr);   	\
	dr = NULL;              	\
	mutex_unlock(&dr_lock); 	\
} while (0)

static volatile int legitimate = LEGITIMATE; // Default state
static volatile unsigned pin;

// The logic in Raspberry Pi BCM2835 writes outputs only when the value must change.
// This is not the nomal behaviour for PLCs, which should write for each scan cycle (each 10ms).
// Thus, for this implementation we have to wait for a very long time (4 secs according to the logic).
// For a typical PLC a wait of 20ms would be enough.
#define WAIT_FOR_LOGIC_W	4500
static void dr_write_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
	// Write instruction of PLC runtime:
	//  - STR R2, [R3] (Opcode 002083e5)
	// If R2 contains a 1 corresponding to current pin, it means that PLC logic is trying
	// to write to the pin while it's in input mode: Pin Control Attack.
	if (regs->ARM_r2 & (1 << PIN_SHIFT(pin))) {
		legitimate = NOT_LEGITIMATE;
		atomic_reset_dr(hw_break); // Remove watchpoint
	}
}

#define WAIT_FOR_LOGIC_R	15
static void dr_read_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
	// Read instruction of PLC runtime:
	//  - LDR	R2, [R3] (00 20 93 E5)
	// Here we are not able to say in which pin the PLC logic is interested (which pins are
	// really considered input), because the register includes 32 pins. Therefore, we assume that
	// PLC logic may have only one input on the watched register, so any read here means Pin Control Attack.
	legitimate = NOT_LEGITIMATE;
	atomic_reset_dr(hw_break); // Remove watchpoint
}

/*
 * Further possible analysis to improve read_handler criterion:
 *
 * After read instruction, I/O value is stored in memory (with following instructions):
 *  - LDR	R3, [FP, #-0x14]
 *  - STR	R2, [R3]
 *  - ...
 *
 * So we put a second read watchpoint on that address (FP - 0x14) and analysed the code again:
 *
 * reset_dr(hw_break);
 * hw_break = set_read_dr(0, (void*)(*(unsigned*)(regs->ARM_fp - 0x14)), dr_further_handler);
 *
 * static void dr_further_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
 * 	// Instructions of PLC runtime which generates this handler:
 * 	//  - LDR	R4, [SB, #0x4C]
 * 	//  - AND	R4, R4, R5
 * 	//  - ...
 * 	// Here R4 is loaded with the I/O value stored before.
 * 	// It seems that R5 is a mask applied to I/O value. But, sadly, this watchpoint fires
 * 	// one time for each different pin (R5 value), even for unused pins. So it's still not possible
 * 	// to know which pin is actually read by the PLC logic.
 * 	// Furthermore, it seems that every pin value is stored to different memory locations,
 * 	// so it's not possible to continue the analysis (it would be useless and unfeasible, because ghostbuster
 * 	// would need to follow each of these addresses with new watchpoints).
 * 	// A collaboration with the PLC runtime authors is needed to continue.
 * }
 */

static inline int is_legitimate(io_detect_t* info, int pid, void* vaddr) {
	target_info_t* tinfo = (target_info_t*)info->target_info;

	pid = 0; // PID not supported for now

	// It is pin multiplexing if either pin mux bits are modified or
	// at least one of pin mux bits is not 0 and pin conf bit is modified
	if ( (tinfo->diff & PIN_MUX_MASK(tinfo->reg_pin)) ||
	     (info->old_val & PIN_MUX_MASK(tinfo->reg_pin)) ) {
		// Pin Multiplexing is never legitimate
		return NOT_LEGITIMATE;
	} else {
		// Pin Configuration: check if PLC logic is conforming with configuration
		if (info->new_val & PIN_CONF_MASK(tinfo->reg_pin)) {
			// Output, operation should be WRITE
			// If read watchpoint is triggered at least once on this pin,
			// then it is Pin Control Attack.
			pin = tinfo->pin; // Save global pin number for watchpoint
			hw_break = set_read_dr(pid, vaddr + LEV_REG(pin), dr_read_handler);
			msleep(WAIT_FOR_LOGIC_R);
		} else {
			// Input, operation should be READ
			// If write watchpoint is triggered at least once on this pin,
			// then it is Pin Control Attack.
			pin = tinfo->pin; // Save global pin number for watchpoint
			hw_break = set_write_dr(pid, vaddr + SET_REG(pin), dr_write_handler);
			msleep(WAIT_FOR_LOGIC_W);
		}
		atomic_reset_dr(hw_break); // Remove watchpoint
		if (legitimate) return LEGITIMATE;
		legitimate = LEGITIMATE; // Reset flag
		return NOT_LEGITIMATE;
	}
}

static inline void update_io_state(io_detect_t* info) {
	target_info_t* tinfo = (target_info_t*)info->target_info;
	*(tinfo->trusted) ^= tinfo->diff;
}

static inline void __restore_io_state(io_detect_t* info) {
	target_info_t* tinfo = (target_info_t*)info->target_info;
	iowrite32(info->new_val ^ tinfo->diff, info->target);
}

#endif
