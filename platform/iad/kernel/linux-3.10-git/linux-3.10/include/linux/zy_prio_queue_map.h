/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2018 Sphairon GmbH (a ZyXEL company)
 */

#ifndef __SPH_PRIO_MAP_H
#define __SPH_PRIO_MAP_H

#include <linux/slab.h>
#include <linux/rculist.h>
#include <linux/seq_file.h>
#include <linux/pkt_sched.h>

struct pq_map_entry {
	struct hlist_node list;
	struct rcu_head rcu;
	unsigned int priority;
	unsigned int phys_queue;
        unsigned int virt_queue;
};

#define PRIO_QUEUE_HASH_SHIFT	3
#define PRIO_QUEUE_HASH_SIZE	(1 << PRIO_QUEUE_HASH_SHIFT)
#define PRIO_QUEUE_HASH_MASK	(PRIO_QUEUE_HASH_SIZE - 1)
#define PRIO_QUEUE_MAX_QUEUES	16

struct pq_map {
	struct hlist_head map[PRIO_QUEUE_HASH_SIZE];
	unsigned int max_queues;
	unsigned int virt_to_phys_queue[PRIO_QUEUE_MAX_QUEUES];
};

static inline unsigned int __pq_map_hashfn(unsigned int idx)
{
	return ((idx >> PRIO_QUEUE_HASH_SHIFT) ^ idx) & PRIO_QUEUE_HASH_MASK;
}

static inline struct pq_map_entry *
__pq_map_alloc(unsigned int priority, unsigned int phys_queue,
		unsigned int virt_queue)
{
	struct pq_map_entry *entry;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return NULL;

	INIT_HLIST_NODE(&entry->list);
	entry->priority = priority;
	entry->phys_queue = phys_queue;
	entry->virt_queue = virt_queue;

	return entry;
}

static inline struct pq_map_entry *
__pq_map_find_prio(struct hlist_head *head, unsigned int priority)
{
	struct pq_map_entry *entry;

	hlist_for_each_entry_rcu(entry, head, list) {
		if (entry->priority == priority)
			return entry;
	}

	return NULL;
}

static inline struct pq_map_entry *
__pq_map_find_queue(struct hlist_head *head, unsigned int queue)
{
	struct pq_map_entry *entry;

	hlist_for_each_entry_rcu(entry, head, list) {
		if (entry->phys_queue == queue)
			return entry;
	}

	return NULL;
}

static inline void pq_map_add_queue(struct pq_map *pqmap, unsigned int queue)
{
	BUG_ON(pqmap == NULL);

	pqmap->virt_to_phys_queue[pqmap->max_queues] = queue;
	pqmap->max_queues++;
}

static inline void pq_map_del_queue(struct pq_map *pqmap, unsigned int queue)
{
	unsigned int i;
	struct hlist_head *head;
	struct pq_map_entry *entry;

	BUG_ON(pqmap == NULL);

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];

		rcu_read_lock();
		entry = __pq_map_find_queue(head, queue);
		rcu_read_unlock();
		if (entry) {
			hlist_del_rcu(&entry->list);
			kfree_rcu(entry, rcu);
		}
	}

	pqmap->max_queues--;
	pqmap->virt_to_phys_queue[pqmap->max_queues] = 0;
}

static inline unsigned int pq_map_get_queue(struct pq_map *pqmap,
						unsigned int priority)
{
	unsigned int hash;
	struct pq_map_entry *entry;
	struct hlist_head *head;

	BUG_ON(pqmap == NULL);

	hash = __pq_map_hashfn(priority);
	head = &pqmap->map[hash];

	/*
	 * no rcu_read_lock() required as this function is only called in
	 * dev_queue_xmit() context which already holds the RCU lock.
	 */
	entry = __pq_map_find_prio(head, priority);
	if (entry)
		return entry->phys_queue;

	return pqmap->max_queues - 1;
}

static inline int pq_map_set_queue(struct pq_map *pqmap,
				unsigned int priority, unsigned int virt_queue)
{
	unsigned int hash, phys_queue;
	struct hlist_head *head;
	struct pq_map_entry *entry, *new_entry;

	BUG_ON(pqmap == NULL);

	if (virt_queue >= pqmap->max_queues) {
		pr_err("%s: virt_queue %u is higher than allowed max queues %u\n",
			__func__, virt_queue, pqmap->max_queues);
		return -EINVAL;
	}

	phys_queue = pqmap->virt_to_phys_queue[virt_queue];

	new_entry = __pq_map_alloc(priority, phys_queue, virt_queue);
	if (!new_entry)
		return -ENOMEM;

	hash = __pq_map_hashfn(priority);
	head = &pqmap->map[hash];

	rcu_read_lock();
	entry = __pq_map_find_prio(head, priority);
	rcu_read_unlock();
	if (entry) {
		hlist_replace_rcu(&entry->list, &new_entry->list);
		kfree_rcu(entry, rcu);
	} else {
		hlist_add_head_rcu(&new_entry->list, head);
	}

	return 0;
}

/*
 * Map priorities to queues in a linear one-to-one fashion
 *
 * Assumptions:
 * queue 0 has highest priority
 * queue N has lowest priority
 * default queue is queue with lowest priority
 */
static inline void pq_map_map_linear(struct pq_map *pqmap)
{
	unsigned int priority, queue;

	BUG_ON(pqmap == NULL);
	BUG_ON(pqmap->max_queues < 1);

	for (queue = 0, priority = pqmap->max_queues - 1;
		queue < pqmap->max_queues;
		queue++, priority--) {
		pq_map_set_queue(pqmap, priority, queue);
	}
}

/*
 * Map priorities according to rt_tos2priority() to lowest four queues
 *
 * Assumptions:
 * queue 0 has highest priority
 * queue N has lowest priority
 * default queue is queue with lowest priority
 */
static inline void pq_map_map_iptos(struct pq_map *pqmap)
{
	BUG_ON(pqmap == NULL);
	BUG_ON(pqmap->max_queues < 4);

	pq_map_set_queue(pqmap, TC_PRIO_BESTEFFORT, pqmap->max_queues - 1);
	pq_map_set_queue(pqmap, TC_PRIO_FILLER, pqmap->max_queues - 1);
	pq_map_set_queue(pqmap, TC_PRIO_BULK, pqmap->max_queues - 2);
	pq_map_set_queue(pqmap, TC_PRIO_INTERACTIVE_BULK, pqmap->max_queues - 3);
	pq_map_set_queue(pqmap, TC_PRIO_INTERACTIVE, pqmap->max_queues - 3);
	pq_map_set_queue(pqmap, TC_PRIO_CONTROL, pqmap->max_queues - 4);
}

static inline void __pq_map_flush(struct hlist_head *head)
{
	struct pq_map_entry *entry;

	hlist_for_each_entry(entry, head, list) {
		hlist_del_rcu(&entry->list);
		kfree_rcu(entry, rcu);
	}
}

static inline void pq_map_flush(struct pq_map *pqmap)
{
	int i;
	struct hlist_head *head;

	BUG_ON(pqmap == NULL);

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];
		__pq_map_flush(head);
	}
}

static inline void pq_map_init(struct pq_map *pqmap, unsigned int max_queues)
{
	unsigned int i;
	struct hlist_head *head;

	BUG_ON(pqmap == NULL);

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];
		INIT_HLIST_HEAD(head);
	}

	for (i = 0; i < max_queues; i++)
		pq_map_add_queue(pqmap, i);
}

static inline void pq_map_release(struct pq_map *pqmap)
{
	int i;
	struct hlist_head *head;

	BUG_ON(pqmap == NULL);

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];
		__pq_map_flush(head);
	}

	rcu_barrier();
}

static inline void pq_map_seq_show(const struct pq_map *pqmap,
					struct seq_file *seq)
{
	int i;
	const struct pq_map_entry *entry;
	const struct hlist_head *head;

	rcu_read_lock();

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];

		hlist_for_each_entry_rcu(entry, head, list) {
			seq_printf(seq, "    %-6u (%x:%03x) -> %u (%u)\n",
				entry->priority,
				(entry->priority >> 16) & 0xffff,
				entry->priority & 0xffff,
				entry->virt_queue, entry->phys_queue);
		}
	}
	seq_puts(seq, "\n");

	rcu_read_unlock();
}

static inline void pq_map_dump(const struct pq_map *pqmap)
{
	int i;
	const struct pq_map_entry *entry;
	const struct hlist_head *head;

	rcu_read_lock();

	for (i = 0; i < PRIO_QUEUE_HASH_SIZE; i++) {
		head = &pqmap->map[i];

		hlist_for_each_entry_rcu(entry, head, list) {
			pr_debug("%-6u (%x:%03x) -> %u\n",
				entry->priority,
				(entry->priority >> 16) & 0xffff,
				entry->priority & 0xffff,
				entry->phys_queue);
		}
	}

	rcu_read_unlock();
}

#endif
