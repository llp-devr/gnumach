/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University.
 * Copyright (c) 1993,1994 The University of Utah and
 * the Computer Systems Laboratory (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON, THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION, AND DISCLAIM ANY LIABILITY
 * OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF
 * THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	ipc/ipc_space.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for IPC spaces of capabilities.
 */

#ifndef	_IPC_IPC_SPACE_H_
#define _IPC_IPC_SPACE_H_

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/mach_types.h>
#include <machine/vm_param.h>
#include <kern/macros.h>
#include <kern/lock.h>
#include <kern/rdxtree.h>
#include <kern/slab.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_types.h>

/*
 *	Every task has a space of IPC capabilities.
 *	IPC operations like send and receive use this space.
 *	IPC kernel calls manipulate the space of the target task.
 *
 *	Every space has a non-NULL is_table with is_table_size entries.
 *	A space may have a NULL is_tree.  is_tree_small records the
 *	number of entries in the tree that, if the table were to grow
 *	to the next larger size, would move from the tree to the table.
 *
 *	is_growing marks when the table is in the process of growing.
 *	When the table is growing, it can't be freed or grown by another
 *	thread, because of krealloc/kmem_realloc's requirements.
 */

typedef unsigned int ipc_space_refs_t;

struct ipc_space {
	decl_simple_lock_data(,is_ref_lock_data)
	ipc_space_refs_t is_references;

	decl_simple_lock_data(,is_lock_data)
	boolean_t is_active;		/* is the space alive? */
	struct rdxtree is_map;		/* a map of entries */
	size_t is_size;			/* number of entries */
	struct rdxtree is_reverse_map;	/* maps objects to entries */
	ipc_entry_t is_free_list;	/* a linked list of free entries */
	size_t is_free_list_size;	/* number of free entries */
#define IS_FREE_LIST_SIZE_LIMIT	64	/* maximum number of entries
					   in the free list */
};


#define	IS_NULL			((ipc_space_t) 0)

extern struct kmem_cache ipc_space_cache;

#define is_alloc()		((ipc_space_t) kmem_cache_alloc(&ipc_space_cache))
#define	is_free(is)		kmem_cache_free(&ipc_space_cache, (vm_offset_t) (is))

extern struct ipc_space *ipc_space_kernel;
extern struct ipc_space *ipc_space_reply;

#define	is_ref_lock_init(is)	simple_lock_init(&(is)->is_ref_lock_data)

#define	ipc_space_reference_macro(is)					\
MACRO_BEGIN								\
	simple_lock(&(is)->is_ref_lock_data);				\
	assert((is)->is_references > 0);				\
	(is)->is_references++;						\
	simple_unlock(&(is)->is_ref_lock_data);				\
MACRO_END

#define	ipc_space_release_macro(is)					\
MACRO_BEGIN								\
	ipc_space_refs_t _refs;						\
									\
	simple_lock(&(is)->is_ref_lock_data);				\
	assert((is)->is_references > 0);				\
	_refs = --(is)->is_references;					\
	simple_unlock(&(is)->is_ref_lock_data);				\
									\
	if (_refs == 0)							\
		is_free(is);						\
MACRO_END

#define	is_lock_init(is)	simple_lock_init(&(is)->is_lock_data)

#define	is_read_lock(is)	simple_lock(&(is)->is_lock_data)
#define is_read_unlock(is)	simple_unlock(&(is)->is_lock_data)

#define	is_write_lock(is)	simple_lock(&(is)->is_lock_data)
#define	is_write_lock_try(is)	simple_lock_try(&(is)->is_lock_data)
#define is_write_unlock(is)	simple_unlock(&(is)->is_lock_data)

#define	is_write_to_read_lock(is)

extern void ipc_space_reference(struct ipc_space *space);
extern void ipc_space_release(struct ipc_space *space);

#define	is_reference(is)	ipc_space_reference(is)
#define	is_release(is)		ipc_space_release(is)

kern_return_t	ipc_space_create(ipc_table_size_t, ipc_space_t *);
kern_return_t	ipc_space_create_special(struct ipc_space **);
void		ipc_space_destroy(struct ipc_space *);

/* Reverse lookups.  */

/* Cast a pointer to a suitable key.  */
#define KEY(X)								\
	({								\
		assert((((unsigned long) (X)) & 0x07) == 0);		\
		((unsigned long long)					\
		 (((unsigned long) (X) - VM_MIN_KERNEL_ADDRESS) >> 3));	\
	})

/* Insert (OBJ, ENTRY) pair into the reverse mapping.  SPACE must
   be write-locked.  */
static inline kern_return_t
ipc_reverse_insert(ipc_space_t space,
		   ipc_object_t obj,
		   ipc_entry_t entry)
{
	assert(space != IS_NULL);
	assert(obj != IO_NULL);
	return (kern_return_t) rdxtree_insert(&space->is_reverse_map,
					      KEY(obj), entry);
}

/* Remove OBJ from the reverse mapping.  SPACE must be
   write-locked.  */
static inline ipc_entry_t
ipc_reverse_remove(ipc_space_t space,
		   ipc_object_t obj)
{
	assert(space != IS_NULL);
	assert(obj != IO_NULL);
	return rdxtree_remove(&space->is_reverse_map, KEY(obj));
}

/* Remove all entries from the reverse mapping.  SPACE must be
   write-locked.  */
static inline void
ipc_reverse_remove_all(ipc_space_t space)
{
	assert(space != IS_NULL);
	rdxtree_remove_all(&space->is_reverse_map);
	assert(space->is_reverse_map.height == 0);
	assert(space->is_reverse_map.root == NULL);
}

/* Return ENTRY related to OBJ, or NULL if no such entry is found in
   the reverse mapping.  SPACE must be read-locked or
   write-locked.  */
static inline ipc_entry_t
ipc_reverse_lookup(ipc_space_t space,
		   ipc_object_t obj)
{
	assert(space != IS_NULL);
	assert(obj != IO_NULL);
	return rdxtree_lookup(&space->is_reverse_map, KEY(obj));
}

#undef KEY

#endif	/* _IPC_IPC_SPACE_H_ */
