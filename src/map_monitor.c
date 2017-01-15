#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mman.h>

#include "io_conf.h"
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

static void* mmap_mem; // Pointer to /dev/mem mmap function (used to recognize /dev/mem requests)

int start_map_monitor(void) {
	mmap_mem = (void*)kallsyms_lookup_name("mmap_mem");

	hook_map_syscalls(hooks, original, free_maps);

	log_info("MAP monitor started\n");
	return 0;
}

static int map_overlaps_io(unsigned long start, unsigned long end) {
	unsigned i;
	unsigned long b_start, b_end;
	for (i = 0; i < PHYS_IO_CONF->blocks; i++) {
		b_start = (unsigned long)PHYS_IO_CONF->addrs[i];
		b_end = b_start + (unsigned long)PHYS_IO_CONF->sizes[i];
		if ( (start >= b_start && start <= b_end) ||
		     (end >= b_start && end <= b_end) )
			return 1; // Overlaps
	}
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
	struct file *file = NULL;
	pid_t pid = current->pid;
	char* comm = current->comm;

	len = PAGE_ALIGN(len);
	
	file = fget(fd);
	if (!file) goto original_mmap2;
	if (file->f_op->mmap == mmap_mem) { // mmap requested on physical memory /dev/mem
		// Check I/O physical address overlap
		start = pgoff << PAGE_SHIFT;
		end = start + len; // end is also pagealigned
		if (map_overlaps_io(start, end)) {
			log_info("mmap2 request: phys[0x%08lx - 0x%08lx] from %s (%d)", start, end, comm, pid);
			handle_mmap(vaddr, mmap2_real, addr, len, prot, flags, fd, pgoff);
			goto mapping_add;
		}
		vaddr = mmap2_real(addr, len, prot, flags, fd, pgoff);
mapping_add:
		if (!IS_ERR_VALUE(vaddr)) {
			if (add_mapping(start, len, vaddr, pid)) {
				log_err("Unable to allocate kernel space for page mappings\n");
				stop_map_monitor();
			}
		}
		if (file) fput(file);
		return vaddr;
	}

	if (file) fput(file);
original_mmap2:
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

	if (!(paddr = get_mapped_phys(addr, pid))) goto original_mremap;

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

	if (!(paddr = get_mapped_phys(addr, pid))) goto original_remap_file_pages;

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

	if (!get_mapped_phys(addr, pid)) goto original_munmap;
	log_info("munmap request: virt[0x%08lx - 0x%08lx] from %s (%d)\n",
	         addr, addr + len, comm, pid);

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
