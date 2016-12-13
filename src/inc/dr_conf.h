#ifndef __DR_CONF_H
#define __DR_CONF_H

/*
 * Debug registers definitions
 */

// Information about debug register change detection
typedef struct {
	const void* new_state; // New debug register state
	const void* old_state; // Old debug register state
	unsigned index;        // Debug register index
} dr_detect_t;

// Debug register change detection handler.
extern void handle_dr_detection(dr_detect_t*);


/************************ DR monitor interface *************************/

/*
 * The following interface should be implemented by the architecture specific header.
 * To optimize the code, all the functions must be defined as static inline.
 *
 * Debug registers (DR) are divided into breakpoint registers and watchpoint registers.
 * Breakpoints react to code events: when an execution flow reaches a given instruction address.
 * Watchpoints react to data events: when an execution flow reads/writes to/from a given address.
 * DRs are usable by attackers to get precise timing of I/O operation performed by the system,
 * by setting their value to the virtual address of the targeted I/O instruction (code) or I/O memory (data).
 * It also enables the attacker to have access to the address space of the targeted application and to use
 * the same virtual addresses. Thus, as further countermeasure to I/O attack, we monitor debug registers.
 *
 * For kernel side DR protection, the implementation should provide a way to count the number of available debug registers,
 * get their current state, compare it with a given state and, if needed, restore a given state.
 * The state of a single debug register can be made of a different number of registers, depending on the architecture.
 *
 * For user side DR protection, the implementation should define the opcodes and the patch_ktext() function to patch kernel text.
 * This is used to disable the DR user interface provided by the kernel.
 * In this way, the access to DRs from user-space is denied.
 */


/********************** Kernel side DR protection **********************/

/*
 * The implementation should define the size (in bytes) needed to store the information
 * about the state of a single debug register. This size will be used by the monitor to allocate
 * the necessary space (state_size * number_of_DR).
 * The number of available DRs is not known here, it must be retrieved by the target processor at runtime.
 */
#define DR_STATE_SIZE      	__DR_STATE_SIZE
#define DR_STATE_TOTAL_SIZE	(__DR_STATE_SIZE * count_drs())

/*
 * The monitor needs to know how many debug registers are available in the current platform, if any.
 * If 0 debug registers are available on the architecture, then the dr_monitor will not be activated.
 *
 * Return: the number of available debug registers (breakpoints + watchpoints)
 */
static inline unsigned count_drs(void);

/*
 * Get the current state of the available debug registers and store it into the given pointer.
 * @state is allocated and freed by the monitor, the implementation should just use it to store the data.
 *
 * @state: pointer to the DR state data structure, _already_ allocated
 */
static inline void get_dr_state(void* state);

/*
 * Read and compare the current DR state with the trusted state.
 * If a mismatch is detected, then handle_dr_detection() should be called,
 * providing the necessary information through a dr_detect_t object.
 *
 * @trusted_state: the trusted state to check the DRs against
 */
static inline void check_dr_state(const void* trusted_state);

/*
 * Restore a debug register to its trusted state.
 *
 * @info: the information about the detected DR change, as filled in by check_dr_state()
 */
static inline void __restore_dr_state(dr_detect_t* info);

#ifdef DR_MONITOR_ACTIVE

#define restore_dr_state(x) 	do {              \
	__restore_dr_state(x);                    \
	log_info("DR state restored\n");  \
} while(0)

#else

#define restore_dr_state(x) 	(void)0

#endif


/*********************** User side DR protection ***********************/

/*
 * The implementation must define the buffers containing specific code to disable the following functions
 * (from "linux/hw_breakpoint.h"), which represent the entry points for user-space access to DRs:
 *
 * - static struct perf_event* register_user_hw_breakpoint(struct perf_event_attr* attr,
 *                                                         perf_overflow_handler_t triggered,
 *                                                         struct task_struct* tsk);
 * - static int modify_user_hw_breakpoint(struct perf_event *bp, struct perf_event_attr *attr);
 * - static void unregister_hw_breakpoint(struct perf_event *bp);
 *
 * The code of these functions will be overwritten with the content of these buffers.
 * The new code should basically disable the functions, just returning immediately in the most
 * efficient way, without breaking kernel code (e.g. setting a proper return value and jumping
 * to the link register should do the trick on most of the architectures).
 *
 * The implementation buffers and their sizes will be used by the monitor according to the macros below.
 * Buffers will not be dereferenced by the monitor, but used like opaque pointers.
 * Their actual type could be decided by the implementation, according to the opcode size.
 */
#define REGISTER_USER_DR_SIZE	__REGISTER_USER_DR_SIZE
#define MODIFY_USER_DR_SIZE  	__MODIFY_USER_DR_SIZE
#define UNREGISTER_DR_SIZE   	__UNREGISTER_DR_SIZE

#define register_user_dr_new	((void*)__register_user_dr_new)
#define modify_user_dr_new  	((void*)__modify_user_dr_new)
#define unregister_dr_new   	((void*)__unregister_dr_new)
/*
 * Implementation example:
 *
 * #define __REGISTER_USER_DR_SIZE	(x) // Here 'x' is the minimum number of bytes needed to disable the function.
 * #define __MODIFY_USER_DR_SIZE  	(y) // Same
 * #define __UNREGISTER_DR_SIZE   	(z) // Again
 *
 * <type> __register_user_dr_new[__REGISTER_USER_DR_SIZE / sizeof(<type>)] = {
 * 	// opcodes here for register_user_hw_breakpoint
 * };
 * 
 * <type> __modify_user_dr_new[__MODIFY_USER_DR_SIZE / sizeof(<type>)] = {
 * 	// opcodes here for modify_user_hw_breakpoint
 * };
 * 
 * <type> __unregister_dr_new[__UNREGISTER_DR_SIZE / sizeof(<type>)] = {
 * 	// opcodes here for unregister_hw_breakpoint
 * };
 *
 * Where <type> has the same size of an opcode, or any size preferred by the implementation.
 */

/*
 * Patch the text of a kernel function pointed by @addr with the given @new_text of @size bytes,
 * after saving the previous code from @addr into @old_text.
 * Pointers are already allocated and are managed by the caller.
 *
 * @addr: the kernel text address to patch
 * @new_text: the text to write starting from @addr
 * @old_text: buffer to save previous text from @addr
 * @size: size of text to save/patch, in bytes
 */
static inline void patch_ktext(void* addr, void* new_text, void* old_text, unsigned size);


/*
 * Include architecture-dependent debug registers header.
 */

#include "dr_impl.h"

#endif
