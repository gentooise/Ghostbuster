#ifndef __IO_CONF_H
#define __IO_CONF_H

#include "defs.h"

/*
 * The following interface should be implemented by the architecture specific header.
 *
 * To optimize the code, all the functions must be defined as static inline.
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
 * The structure will be then accessed through the following pointer,
 * managed by the module (don't use it from the implementation header).
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
 * For each detected change the implementation should fill detect_info_t and notify handle_detection().
 * The information contained into detect_info_t must be enough to eventually restore the trusted value later.
 *
 * @block: the block base address (virtual)
 * @values: the trusted values base address related to this block
 * @index: the index of the block (position inside io_conf->addrs)
 *
 * Return: the number of changes detected in the I/O memory block
 */
static inline int check_addrs_in_block(volatile void* block, const void* values, unsigned index);

/*
 * Writing to I/O memory is also architecture-dependent.
 * The monitor may decide, based on its current policy, to restore values in I/O memory after a detected change.
 * Given a detect_info_t structure related to a particular I/O detection,
 * the implementation should provide a way to restore the I/O value to the trusted one.
 * detect_info_t contains an extra opaque pointer (@target_info) which can be managed (allocated and freed)
 * by the specific implementation, in order to apply the correct access type needed by the target address.
 *
 * @info: detection info pointer as filled by check_addrs_in_block
 */
static inline void restore_value(detect_info_t* info);


/*
 * Include architecture-dependent I/O configuration header.
 */

#ifndef SOC_MODEL
#error "SOC_MODEL not set."
#else

#define QUOTE(x) QUOTE_EXPAND(x)
#define QUOTE_EXPAND(x) #x
#define INCLUDE_FILE(x) QUOTE(x.h)

#include INCLUDE_FILE(SOC_MODEL)

#endif

#endif
