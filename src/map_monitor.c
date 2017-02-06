#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mman.h>

#include "io_monitor.h" // For map_overlaps_io
#include "map_list.h"
#include "map_conf.h"
#include "map_monitor.h"
#include "map_debug.h"

// Syscall hooks
static asmlinkage long my_mmap2(unsigned long addr, unsigned long len,
                                unsigned long prot, unsigned long flags,
                                unsigned long fd, unsigned long pgoff);
static asmlinkage long my_mremap(unsigned long addr,
                                 unsigned long old_len, unsigned long new_len,
                                 unsigned long flags, unsigned long new_addr);
static asmlinkage long my_remap_file_pages(unsigned long addr, unsigned long len,
                                           unsigned long prot, unsigned long pgoff,
                                           unsigned long flags);
static asmlinkage long my_munmap(unsigned long addr, size_t len);

// Exit callback
static void free_maps(pid_t pid);

static void* hooks[HOOKS_COUNT] = {
	my_mmap2,
	my_mremap,
	my_remap_file_pages,
	my_munmap
};
static void* original[HOOKS_COUNT];

#define mmap2_real           	((mmap2_t)original[MMAP2_INDEX])
#define mremap_real          	((mremap_t)original[MREMAP_INDEX])
#define remap_file_pages_real	((remap_file_pages_t)original[REMAP_FILE_PAGES_INDEX])
#define munmap_real          	((munmap_t)original[MUNMAP_INDEX])

int start_map_monitor(void) {
	hook_map_syscalls(hooks, original, free_maps);

	log_info("MAP monitor started\n");
	return 0;
}

/*
 * The following hooks need to duplicate some code contained into the
 * real version of the system calls. This could be solved by integrating
 * Ghostbuster directly into the kernel source.
 */
static asmlinkage long my_mmap2(unsigned long addr, unsigned long len,
                                unsigned long prot, unsigned long flags,
                                unsigned long fd, unsigned long pgoff) {
	unsigned long start, end, vaddr;
	pid_t pid = current->pid;
	char* comm = current->comm;

	len = PAGE_ALIGN(len);
	
	if (is_phys_mem(fd)) {
		// Check I/O physical address overlap
		start = pgoff << PAGE_SHIFT;
		end = start + len; // end is also pagealigned
		if (map_overlaps_io(start, end)) {
			log_info("mmap2 request: phys[0x%08lx - 0x%08lx] from %s (%d)", start, end, comm, pid);
			handle_mmap(vaddr, mmap2_real, addr, len, prot, flags, fd, pgoff);
		} else {
			vaddr = mmap2_real(addr, len, prot, flags, fd, pgoff);
		}

		if (!IS_ERR_VALUE(vaddr)) {
			if (add_mapping(start, len, vaddr, pid)) {
				log_err("Unable to allocate kernel space for page mappings\n");
				stop_map_monitor();
			}
		}
		return vaddr;
	}

	return mmap2_real(addr, len, prot, flags, fd, pgoff);
}

static asmlinkage long my_mremap(unsigned long addr,
                                 unsigned long old_len, unsigned long new_len,
                                 unsigned long flags, unsigned long new_addr) {
	unsigned long paddr, vaddr, end, n_addr;
	pid_t pid = current->pid;
	char* comm = current->comm;

	if (addr & ~PAGE_MASK) goto original_mremap;
	old_len = PAGE_ALIGN(old_len);
	new_len = PAGE_ALIGN(new_len);
	if (flags & MREMAP_FIXED) n_addr = new_addr;
	else n_addr = addr;

	if (!(paddr = get_mapped_phys(addr, pid))) goto original_mremap; // Not referred to physical memory

	if (new_len > old_len) { // Growing is dangerous
		end = paddr + new_len;
		if (map_overlaps_io(paddr, end)) {
			log_info("mremap request: virt[0x%08lx - 0x%08lx] to virt[0x%08lx - 0x%08lx] from %s (%d)",
			         addr, addr + old_len, n_addr, n_addr + new_len, comm, pid);
			handle_mremap(vaddr, mremap_real, addr, old_len, new_len, flags, new_addr);
			goto mapping_update;
		}
	}

	vaddr = mremap_real(addr, old_len, new_len, flags, new_addr);
mapping_update:
	if (!IS_ERR_VALUE(vaddr)) {
		if (update_mapping(addr, old_len, vaddr, new_len, paddr, pid)) {
			log_err("Unable to allocate kernel space for page mappings\n");
			stop_map_monitor();
		}
	}
	return vaddr;
	
original_mremap:
	return mremap_real(addr, old_len, new_len, flags, new_addr);
}

static asmlinkage long my_remap_file_pages(unsigned long addr, unsigned long len,
                                           unsigned long prot, unsigned long pgoff,
                                           unsigned long flags) {
	unsigned long paddr, start, end, res;
	pid_t pid = current->pid;
	char* comm = current->comm;

	addr = addr & PAGE_MASK;
	len = PAGE_ALIGN(len);

	if (!(paddr = get_mapped_phys(addr, pid))) goto original_remap_file_pages; // Not referred to physical memory

	start = pgoff << PAGE_SHIFT;
	end = start + len;
	if (map_overlaps_io(start, end)) {
		log_info("remap_file_pages request: phys[0x%08lx - 0x%08lx] to phys[0x%08lx - 0x%08lx] from %s (%d)",
		         paddr, paddr + len, start, end, comm, pid);
		handle_remap_fp(res, remap_file_pages_real, addr, len, prot, pgoff, flags);
		goto mapping_alter;
	}

	res = remap_file_pages_real(addr, len, prot, pgoff, flags);
mapping_alter:
	if (!res) alter_mapping(addr, start, len, pid);
	return res;

original_remap_file_pages:
	return remap_file_pages_real(addr, len, prot, pgoff, flags);
}

static asmlinkage long my_munmap(unsigned long addr, size_t len) {
	pid_t pid = current->pid;
	char* comm = current->comm;

	if (addr & ~PAGE_MASK) goto original_munmap;
	len = PAGE_ALIGN(len);

	if (!get_mapped_phys(addr, pid)) goto original_munmap; // Not referred to physical memory
	log_info("munmap request: virt[0x%08lx - 0x%08lx] from %s (%d)\n", addr, addr + len, comm, pid);

	delete_mapping(addr, len, pid);

original_munmap:
	return munmap_real(addr, len);
}

static void free_maps(pid_t pid) {
	clean_mappings(pid);
}

void stop_map_monitor(void) {
	restore_map_syscalls();
	log_info("MAP monitor stopped\n");
}
