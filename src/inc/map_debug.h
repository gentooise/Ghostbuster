#ifndef __MAP_DEBUG_H
#define __MAP_DEBUG_H

#include "log.h"

#include <linux/list.h>

/*
 * MAP debug subsystem.
 *
 * In order to simplify debug, this file provides macros to dump
 * the state of the resources monitored by the MAP monitor.
 */

#ifdef MAP_DEBUG // MAP debug subsystem enabled

static void __dump_map_state(struct list_head* head) {
	struct list_head* pos;
	page* cur;
	if (list_empty(head)) {
		log_info("List of active '/dev/mem' mappings is empty.\n");
	} else {
		log_info("List of active '/dev/mem' mappings:\n");
		list_for_each(pos, head) {
			cur = list_entry(pos, page, pages);
			log_info("\tphys[0x%08lx] -> virt[0x%08lx] (pid %d)\n", cur->paddr, cur->vaddr, cur->pid);
			if (pos->next != head) {
				log_info("\t                      |\n");
				log_info("\t                      v\n");
			}
		}
	}
}

#define dump_map_state() do {                	\
	log_info("--- Start MAP dump ---\n");	\
	__dump_map_state(&page_list);        	\
	log_info("--- End MAP dump ---\n");  	\
} while (0)

#else // MAP debug subsystem disabled

#define dump_map_state() 	(void)0

#endif

#endif
