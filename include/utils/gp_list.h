//SPDX-License-Identifier: LGPL-2.0-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_LIST_H
#define GP_LIST_H

#include <core/gp_common.h>

typedef struct gp_list_head {
	struct gp_list_head *next;
} gp_list_head;

typedef struct gp_dlist_head {
	struct gp_dlist_head *next;
	struct gp_dlist_head *prev;
} gp_dlist_head;

typedef struct gp_list {
	gp_list_head *head;
	gp_list_head *tail;
	size_t cnt;
} gp_list;

typedef struct gp_dlist {
	gp_dlist_head *head;
	gp_dlist_head *tail;
	size_t cnt;
} gp_dlist;

#define GP_LIST_ENTRY(ptr, structure, member) \
	GP_CONTAINER_OF(ptr, structure, member)

#define GP_LIST_FOREACH(list, entry) \
	for (entry = (list)->head; entry; entry = entry->next)

#define GP_DLIST_REV_FOREACH(list, entry) \
	for (entry = (list)->tail; entry; entry = entry->prev)

/*
 * @brief Pushes into head of linked list.
 *
 * @list A linked list.
 * @entry An entry to be pushed.
 */
static inline void gp_list_push_head(gp_list *list, gp_list_head *entry)
{
	if (!list->head)
		list->tail = entry;

	entry->next = list->head;
	list->head = entry;

	list->cnt++;
}

/*
 * @brief Pushes into head of double linked list.
 *
 * @list A double linked list.
 * @entry An entry to be pushed.
 */
static inline void gp_dlist_push_head(gp_dlist *list, gp_dlist_head *entry)
{
	gp_list_push_head((gp_list *)list, (gp_list_head*)entry);

	entry->prev = NULL;
	if (entry->next)
		entry->next->prev = entry;
}

/*
 * Internal do not touch.
 */
static inline void gp_list_push_tail_(gp_list *list, gp_list_head *entry)
{
	entry->next = NULL;

	if (!list->head)
		list->head = entry;
	else
		list->tail->next = entry;

	list->cnt++;
}

/*
 * @brief Pushes into tail of linked list.
 *
 * @list A linked list.
 * @entry An entry to be pushed.
 */
static inline void gp_list_push_tail(gp_list *list, gp_list_head *entry)
{
	gp_list_push_tail_(list, entry);

	list->tail = entry;
}

/*
 * @brief Pushes into tail of double linked list.
 *
 * @list A double linked list.
 * @entry An entry to be pushed.
 */
static inline void gp_dlist_push_tail(gp_dlist *list, gp_dlist_head *entry)
{
	gp_list_push_tail_((gp_list *)list, (gp_list_head*)entry);

	entry->prev = list->tail;
	list->tail = entry;
}

/*
 * @brief Pushes an entry into a double linked list after an entry.
 *
 * @list A double linked list.
 * @after The entry to be pushed after.
 * @entry An entry to be pushed.
 */
static inline void gp_dlist_push_after(gp_dlist *list, gp_dlist_head *after, gp_dlist_head *entry)
{
	entry->prev = after;
	entry->next = after->next;

	if (after->next)
		after->next->prev = entry;
	else
		list->tail = entry;

	after->next = entry;

	list->cnt++;
}

/*
 * @brief Pushes an entry into a double linked list before an entry.
 *
 * @list A double linked list.
 * @before The entry to be pushed before.
 * @entry An entry to be pushed.
 */
static inline void gp_dlist_push_before(gp_dlist *list, gp_dlist_head *before, gp_dlist_head *entry)
{
	entry->next = before;
	entry->prev = before->prev;

	if (before->prev)
		before->prev->next = entry;
	else
		list->head = entry;

	before->prev = entry;

	list->cnt++;
}

/*
 * @brief Pops from a head of a linked list.
 *
 * The returned pointer has to be converted to a structure by gp_list_entry().
 *
 * @list A linked list.
 * @return Entry list head or NULL on empty list.
 */
static inline gp_list_head *gp_list_pop_head(gp_list *list)
{
	gp_list_head *ret = list->head;

	if (!ret)
		return NULL;

	if (!ret->next)
		list->tail = NULL;

	list->head = list->head->next;
	list->cnt--;

	return ret;
}

/*
 * @brief Pops from a head of a double linked list.
 *
 * The returned pointer has to be converted to a structure by gp_list_entry().
 *
 * @list A double linked list.
 * @return Entry list head or NULL on empty list.
 */
static inline gp_dlist_head *gp_dlist_pop_head(gp_dlist *list)
{
	gp_dlist_head *ret;

	ret = (gp_dlist_head*)gp_list_pop_head((gp_list*)list);

	if (ret && ret->next)
		ret->next->prev = NULL;

	return ret;
}

/*
 * @brief Pops from a tail of a double linked list.
 *
 * The returned pointer has to be converted to a structure by gp_list_entry().
 *
 * @list A double linked list.
 * @return Entry list head or NULL on empty list.
 */
static inline gp_dlist_head *gp_dlist_pop_tail(gp_dlist *list)
{
	gp_dlist_head *ret = list->tail;

	if (!ret)
		return NULL;

	list->tail = ret->prev;

	if (!ret->prev)
		list->head = NULL;
	else
		ret->prev->next = NULL;

	list->cnt--;

	return ret;
}

/*
 * @brief Removes a entry from a double linked list.
 *
 * The entry must be present in the list!
 *
 * @list A double linked list.
 * @entry An entry to remove.
 */
static inline void gp_dlist_rem(gp_dlist *list, gp_dlist_head *entry)
{
	if (entry->prev)
		entry->prev->next = entry->next;
	else
		list->head = entry->next;

	if (entry->next)
		entry->next->prev = entry->prev;
	else
		list->tail = entry->prev;

	list->cnt--;
}

/*
 * Internal function, do not call.
 */
static inline gp_list_head *gp_list_merge_sort(gp_list_head *head,
                                               int (*cmp)(const void *, const void *))
{
	gp_list_head *middle, *tmp, *ret;

	if (!head || !head->next)
		return head;

	for (middle = tmp = head; tmp; tmp = tmp->next) {
		ret = middle;
		middle = middle->next;
		if (tmp->next)
			tmp = tmp->next;
	}

	ret->next = NULL;
	head = gp_list_merge_sort(head, cmp);
	middle = gp_list_merge_sort(middle, cmp);

	if (cmp(head, middle)) {
		ret = tmp = head;
		head = head->next;
	} else {
		ret = tmp = middle;
		middle = middle->next;
	}

	while (head || middle) {
		while (head && (!middle || cmp(middle, head) <= 0)) {
			tmp->next = head;
			tmp = head;
			head = head->next;
		}

		while (middle && (!head || cmp(head, middle) <= 0)) {
			tmp->next = middle;
			tmp = middle;
			middle = middle->next;
		}
	}

	tmp->next = NULL;

	return ret;
}

/*
 * @brief Sorts a linked list given a compare function.
 *
 * The compare function works the same as a in the qsort() system function,
 * however the pointers passed are pointers to list head structure. Use
 * gp_list_entry() macro to get the pointers to the structures instead.
 *
 * Example compare function would look like:
 *
 * struct foo {
 *	int i;
 *	gp_list_head head;
 * };
 *
 * static int cmp(const void *a, const void *b)
 * {
 *	struct foo *fa = gp_list_entry(a, struct foo, head);
 *	struct foo *fb = gp_list_entry(b, struct foo, head);
 *
 *	return fa->i - fb->i;
 * }
 *
 * @list A linked list.
 * @cmp A compare function.
 */
static inline void gp_list_sort(gp_list *list,
                                int (*cmp)(const void *, const void *))
{
	gp_list_head *tail;

	if (!list->head)
		return;

	list->head = gp_list_merge_sort(list->head, cmp);

	for (tail = list->head; tail->next; tail = tail->next);

	list->tail = tail;
}

/*
 * @brief Sorts a double linked list given a compare function.
 *
 * See gp_list_sort() for detailed description.
 *
 * @list A double linked list.
 * @cmp A compare function.
 */
static inline void gp_dlist_sort(gp_dlist *list,
                                 int (*cmp)(const void *, const void *))
{
	gp_dlist_head *i;

	if (!list->head)
		return;

	list->head = (gp_dlist_head*)gp_list_merge_sort((gp_list_head*)list->head, cmp);

	if (list->head)
		list->head->prev = NULL;

	for (i = list->head; i->next; i = i->next)
		i->next->prev = i;

	list->tail = i;
}

#endif /* GP_LIST_H */
