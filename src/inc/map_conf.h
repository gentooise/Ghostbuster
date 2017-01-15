#ifndef __MAP_CONF_H
#define __MAP_CONF_H


/************************ Map monitor interface ************************/

/*
 * The following interface should be implemented by the architecture specific header.
 *
 * The system provides access for user space programs to the I/O memory by means of the
 * mapping interface. This mapping interface is made of the following functions:
 *
 * - static asmlinkage long mmap2(unsigned long addr, unsigned long len,
 *                                unsigned long prot, unsigned long flags,
 *                                unsigned long fd, unsigned long pgoff);
 * - static asmlinkage long mremap(unsigned long addr,
 *                                 unsigned long old_len, unsigned long new_len,
 *                                 unsigned long flags, unsigned long new_addr);
 * - static asmlinkage long remap_file_pages(unsigned long addr, unsigned long len,
 *                                           unsigned long prot, unsigned long pgoff,
 *                                           unsigned long flags);
 * - static asmlinkage long munmap(unsigned long addr, size_t len);
 *
 * These functions needs to be monitored in order to keep track of who is requesting access
 * to some protected portion of I/O memory.
 *
 * Furthermore, it is needed to have a notification whenever a process is exiting,
 * that is, when the kernel is freeing its address space (inside do_exit).
 * The way to notify it is architecture-dependent, and has to impose the lowest possible overhead.
 * This event must be notified to the provided 'free_maps' callback.
 */

/*
 * Four different hooks are needed, plus a callback for the process termination.
 * Implementation should use these constants and types if needed.
 */

#define MMAP2_INDEX           	0
#define MREMAP_INDEX          	1
#define REMAP_FILE_PAGES_INDEX	2
#define MUNMAP_INDEX          	3

#define HOOKS_COUNT     	4

typedef asmlinkage long (*mmap2_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
typedef asmlinkage long (*mremap_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
typedef asmlinkage long (*remap_file_pages_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
typedef asmlinkage long (*munmap_t)(unsigned long, size_t);

typedef asmlinkage void (*free_maps_t)(int);


/*
 * Hook the given mapping syscalls with the given addresses and store the function pointers to
 * the original syscalls. After the monitor has finished its check on a hooked syscall,
 * it will use these pointers to call back the original kernel syscall to do the job.
 * The implementation should also provide a way to intercept when the kernel frees the
 * address space of a process, and call the free_maps callback provided here (@fm) on that event.
 *
 * @hooks: set of function pointers to replace syscalls with (in the order: mmap2, mremap, remap_file_pages, munmap)
 * @addrs: set of function pointers to store original syscalls into
 * @fm: a function pointer to the free_maps callback
 */

static void hook_map_syscalls(void** hooks, void** addrs, free_maps_t fm);

/*
 * Restore the original mapping syscalls.
 */

static void restore_map_syscalls(void);


#ifdef MAP_MONITOR_ACTIVE

#define deny_mapping(r) do {                                         	\
	log_cont("... denied\n");                                    	\
	r = -EPERM;                                                  	\
} while(0)

#define handle_mmap(r, m, ...)    	deny_mapping(r)
#define handle_mremap(r, m, ...)  	deny_mapping(r)
#define handle_remap_fp(r, m, ...)	deny_mapping(r)

#else

#define handle_mmap(r, m, ...) do {                                  	\
	r = m(__VA_ARGS__);                                          	\
	if (IS_ERR_VALUE(r)) log_cont("... failed (%ld)\n", (long)r);	\
	else log_cont("... mapped to %08lx\n", r);                   	\
} while(0)

#define handle_mremap(r, m, ...) do {                                	\
	r = m(__VA_ARGS__);                                          	\
	if (IS_ERR_VALUE(r)) log_cont("... failed (%ld)\n", (long)r);	\
	else log_cont("... remapped to 0x%08lx\n", r);               	\
} while(0)

#define handle_remap_fp(r, m, a, ...) do {                           	\
	r = m(a, __VA_ARGS__);                                       	\
	if (r) log_cont("... failed (%ld)\n", (long)r);              	\
	else log_cont("... mapped to %08lx\n", a);                   	\
} while(0)

#endif


/*
 * Include architecture-dependent I/O header.
 */

#include "map_impl.h"


#endif
