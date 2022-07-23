// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

static int tabs_new_free(void)
{
	gp_widget *tabs, *ret;
	unsigned int cnt, act;
	const char *labels[] = {
		"label #1",
		"label #2",
	};

	tabs = gp_widget_tabs_new(2, 1, labels, 0);
	if (!tabs) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	cnt = gp_widget_tabs_cnt(tabs);
	if (cnt != 2) {
		tst_msg("gp_widget_tabs_cnt() returned %u expected 2", cnt);
		return TST_FAILED;
	}

	act = gp_widget_tabs_active_get(tabs);
	if (act != 1) {
		tst_msg("gp_widget_tabs_active_get() returned %u expected 1", cnt);
		return TST_FAILED;
	}

	ret = gp_widget_tabs_child_get(tabs, 0);
	if (ret) {
		tst_msg("gp_widget_tabs_child_get(0) returned %p expected NULL", ret);
		return TST_FAILED;
	}

	ret = gp_widget_tabs_child_get(tabs, 1);
	if (ret) {
		tst_msg("gp_widget_tabs_child_get(1) returned %p expected NULL", ret);
		return TST_FAILED;
	}

	if (ret != gp_widget_tabs_active_child_get(tabs)) {
		tst_msg("gp_widget_tabs_active_child_get() returned wrong data");
		return TST_FAILED;
	}

	gp_widget_free(tabs);

	return TST_SUCCESS;
}

static int tabs_add_rem(void)
{
	gp_widget *tabs, *w;
	unsigned int ret;

	tabs = gp_widget_tabs_new(0, 0, NULL, 0);
	if (!tabs) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	ret = gp_widget_tabs_active_get(tabs);
	if (ret) {
		tst_msg("gp_widget_tabs_active_get() returned %u expected 0", ret);
		return TST_FAILED;
	}

	ret = gp_widget_tabs_cnt(tabs);
	if (ret) {
		tst_msg("gp_widget_tabs_cnt() returned %u expected 0", ret);
		return TST_FAILED;
	}

	gp_widget_tabs_tab_append(tabs, "tab", NULL);

	if (gp_widget_tabs_active_get(tabs) != 0) {
		tst_msg("gp_widget_tabs_active_get() returned %u expected 0",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	ret = gp_widget_tabs_cnt(tabs);
	if (ret != 1) {
		tst_msg("gp_widget_tabs_cnt() returned %u expected 1", ret);
		return TST_FAILED;
	}

	w = gp_widget_tabs_tab_rem(tabs, 0);
	if (w) {
		tst_msg("gp_widget_tabs_rem() returned %p expected NULL", w);
		return TST_FAILED;
	}

	ret = gp_widget_tabs_cnt(tabs);
	if (ret) {
		tst_msg("gp_widget_tabs_cnt() returned %u expected 0", ret);
		return TST_FAILED;
	}

	gp_widget_free(tabs);

	return TST_SUCCESS;
}

static int tabs_active_tab(void)
{
	gp_widget *tabs;
	const char *labels[] = {
		"label #1",
		"label #2",
		"label #3",
		"label #4",
	};

	tabs = gp_widget_tabs_new(4, 1, labels, 0);
	if (!tabs) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	if (gp_widget_tabs_active_get(tabs) != 1) {
		tst_msg("Active tab %u expected 1",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	tst_msg("Removing tab 0");

	gp_widget_tabs_tab_rem(tabs, 0);

	if (gp_widget_tabs_active_get(tabs) != 0) {
		tst_msg("Active tab %u expected 0",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	tst_msg("Setting active tab 1");

	gp_widget_tabs_active_set(tabs, 1);

	if (gp_widget_tabs_active_get(tabs) != 1) {
		tst_msg("Active tab %u expected 1",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	tst_msg("Removing tab 1");

	gp_widget_tabs_tab_rem(tabs, 1);

	if (gp_widget_tabs_active_get(tabs) != 0) {
		tst_msg("Active tab %u expected 0",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	tst_msg("Setting active tab 1");

	gp_widget_tabs_active_set(tabs, 1);

	tst_msg("Removing tab 0");

	gp_widget_tabs_tab_rem(tabs, 0);

	if (gp_widget_tabs_active_get(tabs) != 0) {
		tst_msg("Active tab %u expected 0",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	tst_msg("Add tab to 0");

	gp_widget_tabs_tab_ins(tabs, 0, "new tab #1", NULL);

	if (gp_widget_tabs_active_get(tabs) != 1) {
		tst_msg("Active tab %u expected 1",
		        gp_widget_tabs_active_get(tabs));
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int tab_by_child(void)
{
	gp_widget *tabs, *label;
	int ret;
	const char *labels[] = {
		"label #1",
		"label #2",
		"label #3",
		"label #4",
	};

	label = gp_widget_label_new("Label", 0, 0);
	tabs = gp_widget_tabs_new(4, 1, labels, 0);
	if (!tabs || !label) {
		gp_widget_free(tabs);
		gp_widget_free(label);
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	gp_widget_tabs_put(tabs, 1, label);

	tst_msg("Getting tab by child when present");
	ret = gp_widget_tabs_tab_by_child(tabs, label);
	if (ret != 1) {
		tst_msg("gp_widget_tabs_tab_by_child() returned %i expected 1",  ret);
		return TST_FAILED;
	}

	gp_widget_tabs_rem(tabs, 1);

	tst_msg("Getting tab by child when not present");
	ret = gp_widget_tabs_tab_by_child(tabs, label);
	if (ret != -1) {
		tst_msg("gp_widget_tabs_tab_by_child() returned %i expected 1",  ret);
		return TST_FAILED;
	}

	tst_msg("Freeing tabs widget");
	gp_widget_free(label);
	gp_widget_free(tabs);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "tabs testsuite",
	.tests = {
		{.name = "tabs new free",
		 .tst_fn = tabs_new_free,
		 .flags = TST_CHECK_MALLOC},

		{.name = "tabs add rem free",
		 .tst_fn = tabs_add_rem,
		 .flags = TST_CHECK_MALLOC},

		{.name = "tabs active tab",
		 .tst_fn = tabs_active_tab},

		{.name = "tab by child",
		 .tst_fn = tab_by_child,
		 .flags = TST_CHECK_MALLOC},

		{.name = NULL},
	}
};
