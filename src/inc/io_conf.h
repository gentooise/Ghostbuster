#ifndef __IO_CONF_H
#define __IO_CONF_H

/*
 * I/O definitions
 */

// I/O configuration to monitor, modeled as a set of I/O memory blocks.
typedef struct {
	const void** addrs; // Set of address blocks
	const unsigned* sizes; // Size of each block in bytes
	const unsigned blocks; // Number of blocks
	const unsigned size; // Total size
} io_conf_t;

// Information about I/O change detection
typedef struct {
	void* target;     	// Target address
	long new_val;     	// I/O value after detection
	long old_val;     	// I/O value before (trusted)
	void* target_info;	// Extra target info (implementation defined)
} io_detect_t;

// I/O change detection handler.
extern void handle_io_detection(io_detect_t*);


/*
 * The following interface should be implemented by the architecture specific header.
 *
 * To optimize the code, all the functions must be defined as static inline.
 */

/********************************* I/O *********************************/

/*
 * The system has access to I/O pins by using I/O memory.
 * Some address space in I/O memory is designated for configuration registers, used during I/O initialization process.
 * The number and the nature of these registers varies according to the specific architecture.
 * The I/O configuration registers can be leveraged to perform an I/O attack, e.g. by disabling or modifying specific I/O pins.
 *
 * The implementation, according to the target system, must specify which registers should be protected by the monitor,
 * providing low-level functions to get values from these registers, compare their values with trusted ones and restore them if necessary.
 */

/*
 * The implementation must define an io_conf_t object named phys_io_conf, setting all the necessary fields.
 * The structure must be statically initialized and const, so it will be put in the read-only section by the compiler.
 *
 * Declaration example:
 * static const io_conf_t phys_io_conf = {
 * 	<physical_values>
 * };
 *
 * The structure will be then accessed through the following pointer, managed by the module.
 * The pointer can also be dereferenced and used from the implementation header, if preferred,
 * but the value of the pointer must not be modified.
 */
extern const io_conf_t* io_conf;

/*
 * Getting values from I/O memory is architecture-dependent.
 * Thus, must be done by using read_values function below, which needs the virtual base address of each block.
 * The data should be saved into the contiguous memory region pointed by @values, having io_conf->size bytes,
 * and the same pointer must be returned by the function. The buffer is _already_ allocated and managed by the caller,
 * which consider it as an opaque pointer. Thus, every access to it is performed only by using the functions below.
 *
 * To show the content of I/O memory (for debug purposes) the implementation could also provide dump_values,
 * having the same parameter but no return value. Only enabled by using DEBUG compiler flag.
 *
 * @addrs: set of virtual base addresses of I/O blocks
 *
 * Return: pointer to values read from the blocks
 */
static inline void* read_values(volatile void** addrs, void* values);

#ifdef DEBUG
static inline void dump_values(volatile void** addrs);
#endif

/*
 * The implementation should provide a way to iterate over the addresses of each I/O block
 * to compare the actual values in I/O memory with the trusted ones (in @values).
 * The interface allows the implementation to decide size and alignment of each I/O memory access
 * as required by the specific architecture reference manual, thus optimizing the code.
 * Different I/O blocks may contain I/O registers with different sizes, requiring different access types.
 * Therefore, the implementation may need to know which is the specified block (@index).
 * @index also allows to get the size of the block, which doesn't need to be passed (io_conf->io_sizes[index]).
 * For each detected change the implementation should fill detect_info_t and notify handle_io_detection().
 * The information contained into detect_info_t must be enough to eventually restore the trusted value later.
 *
 * @block: the block base address (virtual)
 * @values: the trusted values base address related to this block
 * @index: the index of the block (position inside io_conf->addrs)
 */
static inline void check_addrs_in_block(volatile void* block, const void* values, unsigned index);

/*
 * Writing to I/O memory is also architecture-dependent.
 * The monitor may decide, based on its current policy, to restore values in I/O memory after a detected change.
 * Given a detect_info_t structure related to a particular I/O detection,
 * the implementation should provide a way to restore the I/O value to the trusted one.
 * detect_info_t contains an extra opaque pointer (@target_info) which can be managed (allocated and freed)
 * by the specific implementation, in order to apply the correct access type needed by the target address.
 * The target_info opaque pointer will not be used after a call to restore_value(), so it must be freed here.
 *
 * @info: detection info pointer as filled by check_addrs_in_block
 */
static inline void restore_value(io_detect_t* info);


/*
 * Include architecture-dependent I/O header.
 */

#include "io_impl.h"


#endif
