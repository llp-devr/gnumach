/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon rights
 * to redistribute these changes.
 */
/*
 *	File:	queue.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Type definitions for generic queues.
 *
 */

#ifndef	_KERN_QUEUE_H_
#define	_KERN_QUEUE_H_

#include <kern/lock.h>

/*
 *	Queue of abstract objects.  Queue is maintained
 *	within that object.
 *
 *	Supports fast removal from within the queue.
 *
 *	How to declare a queue of elements of type "foo_t":
 *		In the "*foo_t" type, you must have a field of
 *		type "queue_chain_t" to hold together this queue.
 *		There may be more than one chain through a
 *		"foo_t", for use by different queues.
 *
 *		Declare the queue as a "queue_t" type.
 *
 *		Elements of the queue (of type "foo_t", that is)
 *		are referred to by reference, and cast to type
 *		"queue_entry_t" within this module.
 */

/*
 *	A generic doubly-linked list (queue).
 */

struct queue_entry {
	struct queue_entry	*next;		/* next element */
	struct queue_entry	*prev;		/* previous element */
};

typedef struct queue_entry	*queue_t;
typedef	struct queue_entry	queue_head_t;
typedef	struct queue_entry	queue_chain_t;
typedef	struct queue_entry	*queue_entry_t;

/*
 *	enqueue puts "elt" on the "queue".
 *	dequeue returns the first element in the "queue".
 *	remqueue removes the specified "elt" from the specified "queue".
 */

#define enqueue(queue,elt)	enqueue_tail(queue, elt)
#define	dequeue(queue)		dequeue_head(queue)

void		enqueue_head(queue_t, queue_entry_t);
void		enqueue_tail(queue_t, queue_entry_t);
queue_entry_t	dequeue_head(queue_t);
queue_entry_t	dequeue_tail(queue_t);
void		remqueue(queue_t, queue_entry_t);
void		insque(queue_entry_t, queue_entry_t);

/*
 *	Macro:		queue_assert
 *	Function:
 *		Used by macros to assert that the given argument is a
 *		queue.
 */
#define queue_assert(q)	(void) ((void) (q)->next, (q)->prev)

/*
 *	Macro:		queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *		void queue_init(q)
 *			queue_t		q;	*MODIFIED*
 */
#define	queue_init(q)	((q)->next = (q)->prev = q)

/*
 *	Macro:		queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		queue_entry_t queue_first(q)
 *			queue_t	q;		*IN*
 */
#define	queue_first(q)	(queue_assert(q), (q)->next)

/*
 *	Macro:		queue_next
 *	Function:
 *		Returns the entry after an item in the queue.
 *	Header:
 *		queue_entry_t queue_next(qc)
 *			queue_t qc;
 */
#define	queue_next(qc)	(queue_assert(qc), (qc)->next)

/*
 *	Macro:		queue_last
 *	Function:
 *		Returns the last entry in the queue.
 *	Header:
 *		queue_entry_t queue_last(q)
 *			queue_t	q;		 *IN*
 */
#define	queue_last(q)	(queue_assert(q), (q)->prev)

/*
 *	Macro:		queue_prev
 *	Function:
 *		Returns the entry before an item in the queue.
 *	Header:
 *		queue_entry_t queue_prev(qc)
 *			queue_t qc;
 */
#define	queue_prev(qc)	(queue_assert(qc), (qc)->prev)

/*
 *	Macro:		queue_end
 *	Function:
 *		Tests whether a new entry is really the end of
 *		the queue.
 *	Header:
 *		boolean_t queue_end(q, qe)
 *			queue_t q;
 *			queue_entry_t qe;
 */
#define	queue_end(q, qe)	(queue_assert(q), queue_assert(qe), \
				 (q) == (qe))

/*
 *	Macro:		queue_empty
 *	Function:
 *		Tests whether a queue is empty.
 *	Header:
 *		boolean_t queue_empty(q)
 *			queue_t q;
 */
#define	queue_empty(q)		queue_end((q), queue_first(q))


/*----------------------------------------------------------------*/
/*
 * Macros that operate on generic structures.  The queue
 * chain may be at any location within the structure, and there
 * may be more than one chain.
 */

/*
 *	Macro:		queue_enter
 *	Function:
 *		Insert a new element at the tail of the queue.
 *	Header:
 *		void queue_enter(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter(head, elt, type, field)			\
MACRO_BEGIN							\
	queue_assert(head);					\
	queue_assert(&(elt)->field);				\
	queue_entry_t prev;					\
								\
	prev = (head)->prev;					\
	if ((head) == prev) {					\
		(head)->next = (queue_entry_t) (elt);		\
	}							\
	else {							\
		((type)prev)->field.next = (queue_entry_t)(elt);\
	}							\
	(elt)->field.prev = prev;				\
	(elt)->field.next = head;				\
	(head)->prev = (queue_entry_t) elt;			\
MACRO_END

/*
 *	Macro:		queue_enter_first
 *	Function:
 *		Insert a new element at the head of the queue.
 *	Header:
 *		void queue_enter_first(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter_first(head, elt, type, field)		\
MACRO_BEGIN							\
	queue_assert(head);					\
	queue_assert(&(elt)->field);				\
	queue_entry_t next;					\
								\
	next = (head)->next;					\
	if ((head) == next) {					\
		(head)->prev = (queue_entry_t) (elt);		\
	}							\
	else {							\
		((type)next)->field.prev = (queue_entry_t)(elt);\
	}							\
	(elt)->field.next = next;				\
	(elt)->field.prev = head;				\
	(head)->next = (queue_entry_t) elt;			\
MACRO_END

/*
 *	Macro:		queue_field [internal use only]
 *	Function:
 *		Find the queue_chain_t (or queue_t) for the
 *		given element (thing) in the given queue (head)
 */
#define	queue_field(head, thing, type, field)			\
		(((head) == (thing)) ? (head) : &((type)(thing))->field)

/*
 *	Macro:		queue_remove
 *	Function:
 *		Remove an arbitrary item from the queue.
 *	Header:
 *		void queue_remove(q, qe, type, field)
 *			arguments as in queue_enter
 */
#define	queue_remove(head, elt, type, field)			\
MACRO_BEGIN							\
	queue_assert(head);					\
	queue_assert(&(elt)->field);				\
	queue_entry_t	next, prev;				\
								\
	next = (elt)->field.next;				\
	prev = (elt)->field.prev;				\
								\
	if ((head) == next)					\
		(head)->prev = prev;				\
	else							\
		((type)next)->field.prev = prev;		\
								\
	if ((head) == prev)					\
		(head)->next = next;				\
	else							\
		((type)prev)->field.next = next;		\
MACRO_END

/*
 *	Macro:		queue_remove_first
 *	Function:
 *		Remove and return the entry at the head of
 *		the queue.
 *	Header:
 *		queue_remove_first(head, entry, type, field)
 *		entry is returned by reference
 */
#define	queue_remove_first(head, entry, type, field)		\
MACRO_BEGIN							\
	queue_assert(head);					\
	queue_assert(&(entry)->field);				\
	queue_entry_t	next;					\
								\
	(entry) = (type) ((head)->next);			\
	next = (entry)->field.next;				\
								\
	if ((head) == next)					\
		(head)->prev = (head);				\
	else							\
		((type)(next))->field.prev = (head);		\
	(head)->next = next;					\
MACRO_END

/*
 *	Macro:		queue_remove_last
 *	Function:
 *		Remove and return the entry at the tail of
 *		the queue.
 *	Header:
 *		queue_remove_last(head, entry, type, field)
 *		entry is returned by reference
 */
#define	queue_remove_last(head, entry, type, field)		\
MACRO_BEGIN							\
	queue_assert(head);					\
	queue_assert(&(entry)->field);				\
	queue_entry_t	prev;					\
								\
	(entry) = (type) ((head)->prev);			\
	prev = (entry)->field.prev;				\
								\
	if ((head) == prev)					\
		(head)->next = (head);				\
	else							\
		((type)(prev))->field.next = (head);		\
	(head)->prev = prev;					\
MACRO_END

/*
 *	Macro:		queue_assign
 */
#define	queue_assign(to, from, type, field)			\
MACRO_BEGIN							\
	queue_assert(&(to)->field);				\
	queue_assert(&(from)->field);				\
	((type)((from)->prev))->field.next = (to);		\
	((type)((from)->next))->field.prev = (to);		\
	*to = *from;						\
MACRO_END

/*
 *	Macro:		queue_iterate
 *	Function:
 *		iterate over each item in the queue.
 *		Generates a 'for' loop, setting elt to
 *		each item in turn (by reference).
 *	Header:
 *		queue_iterate(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_iterate(head, elt, type, field)			\
	for ((elt) = (type) queue_first(head);			\
	     !queue_end((head), (queue_entry_t)(elt));		\
	     (elt) = (type) queue_next(&(elt)->field))



/*----------------------------------------------------------------*/
/*
 *	Define macros for queues with locks.
 */
struct mpqueue_head {
	struct queue_entry	head;		/* header for queue */
	struct slock		lock;		/* lock for queue */
};

typedef struct mpqueue_head	mpqueue_head_t;

#define	round_mpq(size)		(size)

#define mpqueue_init(q) \
	MACRO_BEGIN \
		queue_init(&(q)->head); \
		simple_lock_init(&(q)->lock); \
	MACRO_END

#define mpenqueue_tail(q, elt) \
	MACRO_BEGIN \
		simple_lock(&(q)->lock); \
		enqueue_tail(&(q)->head, elt); \
		simple_unlock(&(q)->lock); \
	MACRO_END

#define mpdequeue_head(q, elt) \
	MACRO_BEGIN \
		simple_lock(&(q)->lock); \
		if (queue_empty(&(q)->head)) \
			*(elt) = 0; \
		else \
			*(elt) = dequeue_head(&(q)->head); \
		simple_unlock(&(q)->lock); \
	MACRO_END

/*
 *	Old queue stuff, will go away soon.
 */

#endif	/* _KERN_QUEUE_H_ */
