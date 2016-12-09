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


/*
 * The following interface should be implemented by the architecture specific header.
 *
 * To optimize the code, all the functions must be defined as static inline.
 */

/*************************** Debug registers ***************************/

/*
 * Debug registers (from now on: "DR") are divided into breakpoint registers and watchpoint registers.
 * Breakpoints react to code events: when an execution flow reaches a given instruction address.
 * Watchpoints react to data events: when an execution flow reads/writes to/from a given address.
 * DRs are usable by attackers to get precise timing of I/O operation performed by the system,
 * by setting their value to the virtual address of the targeted I/O instruction (code) or I/O memory (data).
 * It also enables the attacker to have access to the address space of the targeted application and to use
 * the same virtual addresses. Thus, as further countermeasure to I/O attack, we monitor debug registers as well.
 * This part of the monitor is used to protect debug registers from kernel side access.
 *
 * Each single debug register can be made of different registers depending on the architecture.
 * The implementation should provide a way to count the number of available debug registers,
 * get their current state, compare it with a given state and, if needed, restore a given state.
 */


/*
 * The implementation should define the size (in bytes) needed to store all the information
 * about the state of a single debug register. This size will be used by the monitor to allocate
 * the necessary space (state_size * number_of_DR).
 * The number of available DRs is not known here, it must be retrieved by the target processor at runtime.
 */
#define DR_STATE_SIZE      	__DR_STATE_SIZE
#define DR_STATE_TOTAL_SIZE	(DR_STATE_TOTAL_SIZE * count_drs())

/*
 * The monitor needs to know how many debug registers are available in the current platform, if any.
 * If 0 debug registers are available on the architecture, then the dr_monitor will not be activated.
 *
 * Return: the number of available debug registers (breakpoints + watchpoints)
 */
static inline unsigned count_drs(void);

/*
 * Get the current state of available debug registers. This is called the first time to get trusted values.
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
 * @info: the information about the detected DR change
 */
static inline void restore_dr_state(dr_detect_t* info);


/*
 * Include architecture-dependent debug registers header.
 */

#include "dr_impl.h"

#endif
