// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_list.h>

#include "tst_test.h"

struct list_item {
	int i;
	gp_list_head head;
};

static int list_cmp(const void *a, const void *b)
{
	const struct list_item *la = GP_LIST_ENTRY(a, struct list_item, head);
	const struct list_item *lb = GP_LIST_ENTRY(b, struct list_item, head);

	return la->i < lb->i;
}

static int list_cmp_rev(const void *a, const void *b)
{
	const struct list_item *la = GP_LIST_ENTRY(a, struct list_item, head);
	const struct list_item *lb = GP_LIST_ENTRY(b, struct list_item, head);

	return la->i > lb->i;
}

static void dump_list(gp_list *list)
{
	gp_list_head *i;

	GP_LIST_FOREACH(list, i) {
		struct list_item *j = GP_LIST_ENTRY(i, struct list_item, head);

		printf("%i ", j->i);
	}

	printf("\n");
}

int list_test(void)
{
	gp_list list = {};
	struct list_item items[10];
	gp_list_head *j, *tmp;
	int i;

	tst_msg("Calling sort and pop on empty list");

	gp_list_sort(&list, list_cmp);
	gp_list_pop_head(&list);

	tst_msg("Initializing list=0..9");

	for (i = 0; i < 10; i++) {
		items[i].i = i;
		gp_list_push_tail(&list, &items[i].head);
	}

	if (list.cnt != 10) {
		tst_msg("Wrong list elemenet count %zu expected 10", list.cnt);
		return TST_FAILED;
	}

	i = 0;

	GP_LIST_FOREACH(&list, j) {
		struct list_item *it = GP_LIST_ENTRY(j, struct list_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
			dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	tst_msg("List head pop list=1..9");

	tmp = gp_list_pop_head(&list);

	if (list.cnt != 9) {
		tst_msg("Wrong list elemenet count %zu expected 9", list.cnt);
		return TST_FAILED;
	}

	i = 1;

	GP_LIST_FOREACH(&list, j) {
		struct list_item *it = GP_LIST_ENTRY(j, struct list_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
			dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	tst_msg("Push removed entry to tail list=1..9,0");

	gp_list_push_tail(&list, tmp);

	if (list.tail != tmp) {
		tst_msg("Wrong tail pointer %p, expected %p\n",
			list.tail, tmp);
		return TST_FAILED;
	}

	tst_msg("Sort list=0..9");

	gp_list_sort(&list, list_cmp);

	i = 0;

	GP_LIST_FOREACH(&list, j) {
		struct list_item *it = GP_LIST_ENTRY(j, struct list_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
			dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	if (list.tail != &items[9].head) {
		tst_msg("Wrong tail pointer %p, expected %p\n",
			list.tail, &items[9].head);
		return TST_FAILED;
	}

	tst_msg("Sort reverse list=9..0");

	gp_list_sort(&list, list_cmp_rev);

	if (list.tail != &items[0].head) {
		tst_msg("Wrong tail pointer %p, expected %p\n",
			list.tail, &items[0].head);
		return TST_FAILED;
	}

	tst_msg("Poping whole list");

	i = 9;

	while ((j = gp_list_pop_head(&list))) {
		struct list_item *it = GP_LIST_ENTRY(j, struct list_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
			dump_list(&list);
			return TST_FAILED;
		}

		i--;
	}

	if (list.cnt) {
		tst_msg("Wrong list elemenet count %zu expected 0", list.cnt);
		return TST_FAILED;
	}

	tst_msg("Initializing list=9..0");

	for (i = 0; i < 10; i++) {
		items[i].i = i;
		gp_list_push_head(&list, &items[i].head);
	}

	if (list.cnt != 10) {
		tst_msg("Wrong list elemenet count %zu expected 10", list.cnt);
		return TST_FAILED;
	}

	tst_msg("Poping whole list");

	i = 9;

	while ((j = gp_list_pop_head(&list))) {
		struct list_item *it = GP_LIST_ENTRY(j, struct list_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
			dump_list(&list);
			return TST_FAILED;
		}

		i--;
	}

	if (list.cnt) {
		tst_msg("Wrong list elemenet count %zu expected 0", list.cnt);
		return TST_FAILED;
	}

	tst_msg("List test passed");
	return TST_PASSED;
}

struct dlist_item {
	int i;
	gp_dlist_head head;
};

int dlist_test(void)
{
	gp_dlist list = {};
	struct dlist_item items[10];
	gp_dlist_head *j;
	int i;

	tst_msg("Calling sort and pop on empty list");

	gp_dlist_sort(&list, list_cmp);
	gp_dlist_pop_head(&list);
	gp_dlist_pop_tail(&list);

	tst_msg("Initializing list=0..9");

	for (i = 0; i < 10; i++) {
		items[i].i = i;
		gp_dlist_push_tail(&list, &items[i].head);
	}

	if (list.cnt != 10) {
		tst_msg("Wrong list elemenet count %zu expected 10", list.cnt);
		return TST_FAILED;
	}

	i = 0;

	GP_LIST_FOREACH(&list, j) {
		struct dlist_item *it = GP_LIST_ENTRY(j, struct dlist_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
		//	dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	i = 9;

	GP_DLIST_REV_FOREACH(&list, j) {
		struct dlist_item *it = GP_LIST_ENTRY(j, struct dlist_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
		//	dump_list(&list);
			return TST_FAILED;
		}

		i--;
	}

	tst_msg("Sort reverse list=9..0");
	gp_dlist_sort(&list, list_cmp_rev);

	i = 0;

	GP_DLIST_REV_FOREACH(&list, j) {
		struct dlist_item *it = GP_LIST_ENTRY(j, struct dlist_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
		//	dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	tst_msg("Poping whole list");

	i = 9;

	while ((j = gp_dlist_pop_head(&list))) {
		struct dlist_item *it = GP_LIST_ENTRY(j, struct dlist_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
		//	dump_list(&list);
			return TST_FAILED;
		}

		i--;
	}

	if (list.cnt) {
		tst_msg("Wrong list elemenet count %zu expected 0", list.cnt);
		return TST_FAILED;
	}

	tst_msg("Initializing list=9..0");

	for (i = 0; i < 10; i++) {
		items[i].i = i;
		gp_dlist_push_head(&list, &items[i].head);
	}

	tst_msg("Poping whole list");

	i = 0;

	while ((j = gp_dlist_pop_tail(&list))) {
		struct dlist_item *it = GP_LIST_ENTRY(j, struct dlist_item, head);

		if (it->i != i) {
			tst_msg("Wrong item %i, expected %i", it->i, i);
		//	dump_list(&list);
			return TST_FAILED;
		}

		i++;
	}

	if (list.cnt) {
		tst_msg("Wrong list elemenet count %zu expected 0", list.cnt);
		return TST_FAILED;
	}

	tst_msg("Initializing list=0..9");

	for (i = 0; i < 10; i++) {
		items[i].i = i;
		gp_dlist_push_tail(&list, &items[i].head);
	}

	tst_msg("Removing entries in random order");

	gp_dlist_rem(&list, &items[0].head);
	gp_dlist_rem(&list, &items[9].head);
	gp_dlist_rem(&list, &items[5].head);
	gp_dlist_rem(&list, &items[6].head);
	gp_dlist_rem(&list, &items[4].head);
	gp_dlist_rem(&list, &items[7].head);
	gp_dlist_rem(&list, &items[8].head);
	gp_dlist_rem(&list, &items[2].head);
	gp_dlist_rem(&list, &items[3].head);
	gp_dlist_rem(&list, &items[1].head);

	if (list.cnt) {
		tst_msg("Wrong list elemenet count %zu expected 0", list.cnt);
		return TST_FAILED;
	}

	if (list.head || list.tail) {
		tst_msg("Non-null head %p or tail %p on empty list",
		        list.head, list.tail);
		return TST_FAILED;
	}

	tst_msg("List test passed");
	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "list testsuite",
	.tests = {
		{.name = "list test",
		 .tst_fn = list_test},

		{.name = "double linked list test",
		 .tst_fn = dlist_test},

		{}
	}
};
