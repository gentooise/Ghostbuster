#ifndef __MAP_LIST_H
#define __MAP_LIST_H

#include <linux/list.h>
#include <linux/sched.h>
#include <linux/slab.h>

// Information about I/O mapping requests detected.
// An I/O mapping is detected by checking whether the requested block overlaps
// with at least one of the blocks defined by the I/O monitor.
// Mappings are stored and handled at the page level.
// Each detected mapped page contains the physical address, the virtual address
// and the pid of the process who has requested the mapping.
typedef struct {
	struct list_head pages; // List of current mappings under detection
	unsigned long paddr; // The start physical address requested
	unsigned long vaddr; // The virtual address the request has been mapped into
	pid_t pid; // The process who has requested the mapping
} page;

// Mapped pages are stored into a global list (@page_list).
// We assume that the number of processes normally interested in I/O mappings is quite low,
// so a global list containing mapped pages of these processes together performs well enough.
// If that is not the case, this data structure could be improved (e.g. by using hashtable based on process ids).
LIST_HEAD(page_list); // struct list_head page_list = ...;
DEFINE_MUTEX(page_list_lock); // To protect the list from concurrent accesses

#include "map_debug.h"

#define add_page(p)          	list_add_tail(&((p)->pages), &page_list)
#define add_page_after(p, h) 	list_add(&((p)->pages), &((h)->pages))
#define add_page_before(p, h)	list_add_tail(&((p)->pages), &((h)->pages))
#define del_page(p)          	list_del(&((p)->pages))
#define next_page(p)         	list_entry((p)->pages.next, page, pages)

// Mutex already held by the caller
static inline page* find_page_by_paddr(unsigned long paddr, pid_t pid) {
	struct list_head* pos;
	page* cur;
	list_for_each(pos, &page_list) {
		cur = list_entry(pos, page, pages);
		if (cur->pid == pid && cur->paddr == paddr)
			return cur;
	}
	return NULL;
}

// Mutex already held by the caller
static inline page* find_page_by_vaddr(unsigned long vaddr, pid_t pid) {
	struct list_head* pos;
	page* cur;
	list_for_each(pos, &page_list) {
		cur = list_entry(pos, page, pages);
		if (cur->pid == pid && cur->vaddr == vaddr)
			return cur;
	}
	return NULL;
}

/*
 * Invariant: pages belonging to the same mapped block are contiguous inside page_list.
 * Thus, contiguous virtual pages are contiguous into our list as well.
 * This includes also the case when mremap "splits" a mapped block in two parts,
 * creating a hole in virtual address space. The situation is represented below:
 *
 *               physical memory               virtual memory
 *               |            |                |            |
 *    0x20200000 |------------|   (1) mmap2    |            |
 *               |            | -------------> |------------| 0xbf6e4000
 *    0x20202000 |············| ----           |            |
 *    0x20203000 |············|    |           |············|
 *    0x20204000 |------------|    |           |············|
 *               |            |    |           |------------|
 *               |            |    |   (2)     |            |
 *               |            |    | mremap    |            |
 *               |            |    ----------> |------------| 0xce546000 
 *               |            |                |------------|
 *               |            |                |            |
 *               |            |                |            |
 *               |            |                |            |
 *
 * (1) mmap() maps 4 pages starting from 0x20200000 into vmem -> 4 contiguous nodes into the list
 *                  __________     __________     __________     __________
 *     page_list:  |0xbf6e4000|-->|0xbf6e5000|-->|0xbf6e6000|-->|0xbf6e7000|
 *                 |__________|   |__________|   |__________|   |__________|
 *
 * (2) mremap() remaps 1 page starting from 0x20202000 into a different vmem address,
 *     producing a hole in the previous mapping. The result is having three different mapped blocks.
 *                  __________     __________     __________     __________
 *     page_list:  |0xbf6e4000|-->|0xbf6e5000|-->|0xce546000|-->|0xbf6e7000|
 *                 |__________|   |__________|   |__________|   |__________|
 *
 */
static inline int add_mapping(unsigned long paddr, unsigned long len, unsigned long vaddr, pid_t pid) {
	unsigned long pages;
	unsigned i;
	page* p;
	int res = 0;

	pages = len / PAGE_SIZE;
	mutex_lock(&page_list_lock);
	for (i = 0; i < pages; i++, paddr += PAGE_SIZE, vaddr += PAGE_SIZE) {
		// If the page is already mapped for the process
		// the old mapping is overwritten (as mmap2 does).
		// Otherwise allocate a new page.
		if (!(p = find_page_by_vaddr(vaddr, pid))) {
			p = kmalloc(sizeof(page), GFP_KERNEL);
			if (!p) {
				res = -ENOMEM;
				goto unlock_and_return;
			}
		}
		p->paddr = paddr;
		p->vaddr = vaddr;
		p->pid = pid;
		add_page(p);
	}

unlock_and_return:
	dump_map_state();
	mutex_unlock(&page_list_lock);
	return res;
}

static inline unsigned long get_mapped_phys(unsigned long vaddr, pid_t pid) {
	unsigned long paddr = 0;
	page* p;

	mutex_lock(&page_list_lock);
	p = find_page_by_vaddr(vaddr, pid);
	if (p) paddr = p->paddr;
	mutex_unlock(&page_list_lock);

	return paddr;
}

static inline int update_mapping(unsigned long vaddr, unsigned long len, unsigned long new_vaddr, unsigned long new_len, unsigned long paddr, pid_t pid) {
	unsigned long pages, new_pages, min_pages;
	unsigned i;
	page *p, *tmp;
	int res = 0;

	pages = len / PAGE_SIZE;
	new_pages = new_len / PAGE_SIZE;
	min_pages = min(pages, new_pages);

	mutex_lock(&page_list_lock);
	p = find_page_by_vaddr(vaddr, pid);
	for (i = 0; i < min_pages; i++, new_vaddr += PAGE_SIZE, paddr += PAGE_SIZE) {
		p->vaddr = new_vaddr;
		p = next_page(p);
	}
	// delete exceeding pages...
	for ( ; min_pages < pages; min_pages++) {
		tmp = p;
		p = next_page(p);
		del_page(tmp);
		kfree(tmp);
	}
	// ... or add missing ones
	for ( ; min_pages < new_pages; min_pages++, new_vaddr += PAGE_SIZE, paddr += PAGE_SIZE) {
		tmp = kmalloc(sizeof(page), GFP_KERNEL);
		if (!tmp) {
			res = -ENOMEM;
			goto unlock_and_return;
		}
		tmp->paddr = paddr;
		tmp->vaddr = new_vaddr;
		add_page_before(tmp, p);
	}

unlock_and_return:
	dump_map_state();
	mutex_unlock(&page_list_lock);
	return res;
}

static inline void alter_mapping(unsigned long vaddr, unsigned long paddr, unsigned long len, pid_t pid) {
	unsigned long pages = len / PAGE_SIZE;
	unsigned i;
	page* p;

	mutex_lock(&page_list_lock);
	p = find_page_by_vaddr(vaddr, pid);
	for (i = 0; i < pages; i++, paddr += PAGE_SIZE) {
		p->paddr = paddr;
	}
	dump_map_state();
	mutex_unlock(&page_list_lock);
}

static inline void delete_mapping(unsigned long vaddr, unsigned long len, pid_t pid) {
	unsigned long pages;
	unsigned i;
	page *p, *tmp;

	pages = len / PAGE_SIZE;

	mutex_lock(&page_list_lock);
	p = find_page_by_vaddr(vaddr, pid);
	if (p) {
		for (i = 0; i < pages; i++) {
			tmp = p;
			p = next_page(p);
			del_page(tmp);
			kfree(tmp);
		}

		dump_map_state();
	}
	mutex_unlock(&page_list_lock);
}

static inline void clean_mappings(pid_t pid) {
	struct list_head *pos, *tmp;
	page* cur;

	mutex_lock(&page_list_lock);
	list_for_each_safe(pos, tmp, &page_list) {
		cur = list_entry(pos, page, pages);
		if (cur->pid == pid) {
			del_page(cur);
			kfree(cur);
		}	
	}
	dump_map_state();
	mutex_unlock(&page_list_lock);
}

#endif
