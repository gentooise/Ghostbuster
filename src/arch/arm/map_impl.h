#ifndef __MAP_IMPL_H
#define __MAP_IMPL_H

#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include <linux/stop_machine.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>

#include <linux/notifier.h>
#include <asm/thread_info.h>
#include <asm/thread_notify.h>

static void** sys_call_table;
static void* original_syscalls[HOOKS_COUNT];
static free_maps_t free_maps_callback;

static int exit_notifier(struct notifier_block *self, unsigned long cmd, void *t) {
	struct thread_info* thread = t;

	if (cmd == THREAD_NOTIFY_EXIT && (thread->task->flags & PF_EXITING)) {
		free_maps_callback(thread->task->pid);
	}

	return NOTIFY_DONE;
}

static struct notifier_block exit_notifier_block = {
	.notifier_call  = exit_notifier,
};

static int __patch_map_syscalls(void* arg) {
	void** addrs = (void**)arg;

	sys_call_table[__NR_mmap2] = addrs[MMAP2_INDEX];
	sys_call_table[__NR_mremap] = addrs[MREMAP_INDEX];
	sys_call_table[__NR_remap_file_pages] = addrs[REMAP_FILE_PAGES_INDEX];
	sys_call_table[__NR_munmap] = addrs[MUNMAP_INDEX];

	flush_cache_all();
	flush_tlb_all();
	return 0;
}

static void patch_map_syscalls(void** addrs) {
	stop_machine(__patch_map_syscalls, (void*)addrs, NULL);
}

static void hook_map_syscalls(void** hooks, void** addrs, free_maps_t fm) {
	sys_call_table = (void**)kallsyms_lookup_name("sys_call_table");

	// Save original system calls
	original_syscalls[MMAP2_INDEX] = sys_call_table[__NR_mmap2];
	original_syscalls[MREMAP_INDEX] = sys_call_table[__NR_mremap];
	original_syscalls[REMAP_FILE_PAGES_INDEX] = sys_call_table[__NR_remap_file_pages];
	original_syscalls[MUNMAP_INDEX] = sys_call_table[__NR_munmap];

	// mmap2 has an atypical parameter convention in ARM,
	// 'sys_mmap_pgoff' should be called instead of the original pointer in syscall table.
	addrs[MMAP2_INDEX] = (void*)kallsyms_lookup_name("sys_mmap_pgoff");
	addrs[MREMAP_INDEX] = original_syscalls[MREMAP_INDEX];
	addrs[REMAP_FILE_PAGES_INDEX] = original_syscalls[REMAP_FILE_PAGES_INDEX];
	addrs[MUNMAP_INDEX] = original_syscalls[MUNMAP_INDEX];

	// Place our hooks
	free_maps_callback = fm;
	patch_map_syscalls(hooks);
	// Use the notifier implemented for ARM (<asm/thread_notify.h>).
	// Overhead: normal function call.
	thread_register_notifier(&exit_notifier_block);
}

static void restore_map_syscalls(void) {
	// Remove our hooks
	thread_unregister_notifier(&exit_notifier_block);
	patch_map_syscalls(original_syscalls);
}


#endif
