#ifndef __MAP_MONITOR_H
#define __MAP_MONITOR_H

/*
 * This monitor is responsible to watch over I/O address mapping requests.
 * An address mapping provides an association between physical addresses and virtual addresses,
 * making use of the Memory Management Unit (MMU). If the target architecture
 * is working with MMU disabled, or does not have it at all, then this monitor is not needed
 * and must be disabled.
 *
 * The target of the map monitor are all the mapping requests whose physical address range
 * overlaps at least one of the configuration addresses monitored by the I/O monitor.
 * Such requests suggest that someone is trying to map one of the I/O addresses that should be protected,
 * and he will soon have access to it after mapping.
 * If the attacker tries to write to one of those addresses, it will be (hopefully) detected
 * by the io_monitor anyway, so the map monitor represents a further countermeasure useful not
 * only for detecting the attack as early as possible, but also for accounting, giving information
 * about WHO is trying to access I/O configuration addresses.
 * 
 * I/O mappings can be requested both from kernel side and user side.
 *  1. Kernel: through ioremap, or by manually put mappings into page table entry (that is what ioremap does).
 *  2. User: mmap2, mremap, and remap_file_pages syscalls (*),
 *     which operates either on a file (mmap2) or on a previous mapped virtual address (mremap, remap_file_pages).
 *     The file that gives user space access to the physical memory is "/dev/mem".
 *     Moreover, to complete the monitor and clean current mappings when requested, munmap must be monitored too.
 *     (*) mmap syscall has been superseded by mmap2 since kernel 2.4, so it is not considered here.
 *
 * Since ioremap can be easily bypassed from kernel side, there is no point in monitoring it.
 * In any case, as pointed out above, either if the attacker uses ioremap or any other way
 * to get access to one of the I/O configuration addresses, it will be detected by the I/O monitor.
 *
 * Therefore, we monitor user mapping requests referred to '/dev/mem', coming from the following syscalls:
 *  - mmap2 (mmap_pgoff): http://man7.org/linux/man-pages/man2/mmap2.2.html
 *  - mremap: http://man7.org/linux/man-pages/man2/mremap.2.html
 *  - remap_file_pages: http://man7.org/linux/man-pages/man2/remap_file_pages.2.html
 *  - munmap: http://man7.org/linux/man-pages/man2/mmap.2.html
 * If the monitor is passive (default), it stores information about which user programs currently have
 * I/O addresses mapped in their own virtual address space, and which pages are mapped.
 * If it is in active mode, it prevents any process to request overlapping mappings, and logs
 * anyone who attempts to map the I/O.
 *
 * Furthermore, in order to clean-up mapped pages, we need to receive notification when
 * a process terminates and the kernel is freeing all its data.
 * When a user process exits, if munmap has not been properly called before,
 * the kernel automatically unmaps all the user space. Thus, we need to remove mapped pages as well.
 * Monitoring exit system calls (exit and exit_group) is not enough on Linux, because a process
 * may also terminate because of an unhandled signal (such as SIGSEGV). In these cases the kernel
 * automatically destroys the task without going through the above syscalls.
 * In any case, do_exit() is used to free all the memory related to an exiting task. As widely suggested,
 * we could set a kprobe on do_exit() call to achieve this goal, but unfortunately it imposes too much overhead
 * for our target system (it uses software breakpoint). Therefore, we put this task into
 * the architecture-dependent part of the MAP monitor, to allow having different and efficient implementations.
 * See actual implementations inside 'arch/<ARCH>' directories.
 */

#ifdef MAP_MONITOR_ENABLED

int start_map_monitor(void);

void stop_map_monitor(void);

#else

#define start_map_monitor()	0
#define stop_map_monitor() 	(void)0

#endif

#endif
