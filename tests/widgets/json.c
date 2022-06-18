// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "tst_test.h"
#include "common.h"

struct tcase {
	const char *json;
	unsigned int align;
	const char *uid;
	int on_event;
	int no_shrink;
};

static int invalid_empty(struct tcase *t)
{
	gp_widget *ret;

	ret = gp_widget_from_json_str(t->json, NULL, NULL);
	if (ret) {
		tst_msg("Parser was successful!");
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int ev_type;

int on_event_fn(gp_widget_event *ev)
{
	if (!ev)
		return 42;

	ev_type = ev->type;
	return 0;
}

static int load_json(struct tcase *t)
{
	gp_widget *ret;
	gp_htable *uids = NULL;

	ev_type = 0;

	ret = gp_widget_from_json_str(t->json, NULL, &uids);
	if (!ret) {
		tst_msg("Parser failed!");
		return TST_FAILED;
	}

	if (t->align != ret->align) {
		tst_msg("Wrong alignment %u expected %u", ret->align, t->align);
		return TST_FAILED;
	}

	if (t->no_shrink != ret->no_shrink) {
		tst_msg("Wrong no shrink attribute %i expected %i", ret->no_shrink, t->no_shrink);
		return TST_FAILED;
	}

	if (t->uid) {
		gp_widget *self = gp_htable_get(uids, t->uid);

		if (!self || self != ret) {
			tst_msg("Wrong pointer for uid '%s' (%p)", t->uid, self);
			return TST_FAILED;
		}
	}

	if (t->on_event) {
		if (!ret->on_event) {
			tst_msg("Function on_event not set!");
			return TST_FAILED;
		}

		if (ev_type != GP_WIDGET_EVENT_NEW) {
			tst_msg("Widget JSON constructor not called!");
			return TST_FAILED;
		}

		if (ret->on_event(NULL) != 42) {
			tst_msg("Wrong return from on_event function!");
			return TST_FAILED;
		}
	} else {
		if (ret->on_event) {
			tst_msg("Function on_event set!");
			return TST_FAILED;
		}
	}

	gp_widget_free(ret);
	gp_htable_free(uids);

	return TST_SUCCESS;
}

static struct tcase missing_info = {
	.json = "{}",
};

static struct tcase missing_version = {
	.json = "{\"info\":{}}",
};

static struct tcase invalid_version = {
	.json = "{\"info\": {\"version\": -42}}",
};

static struct tcase version_not_number = {
	.json = "{\"info\": {\"version\": \"hello world\"}}",
};

static struct tcase empty = {
	.json = "{\"info\": {\"version\": 1}}",
};

static struct tcase not_object = {
	.json = "[]",
};

static struct tcase invalid_widget_type = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
		" \"layout\": {\"type\": \"this_is_not_a_widget_type\"}}",
};

static struct tcase widget_type_not_string = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
		" \"layout\": {\"type\": 3.14}}",
};

static struct tcase widget_uid_not_string = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
		" \"layout\": {\"uid\": 3.14}}",
};

static struct tcase widget_default_align = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
		" \"layout\": {\"widgets\": [{}]}}",
};

static struct tcase widget_align_fill = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"align\": \"fill\", \"widgets\": [{}]}}",
	.align = GP_HFILL | GP_VFILL,
};

static struct tcase widget_align_left = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"halign\": \"left\", \"widgets\": [{}]}}",
	.align = GP_LEFT,
};

static struct tcase widget_align_bottom = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"valign\": \"bottom\", \"widgets\": [{}]}}",
	.align = GP_BOTTOM,
};

static struct tcase widget_uid = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"uid\": \"this_is_uid\", \"widgets\": [{}]}}",
	.uid = "this_is_uid"
};

static struct tcase widget_missing_on_event = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"on_event\": \"on_event_does_not_exists\", \"widgets\": [{}]}}",
};

static struct tcase widget_on_event = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"on_event\": \"on_event_fn\", \"widgets\": [{}]}}",
	.on_event = 1,
};

static struct tcase widget_no_shrink = {
	.json = "{\"info\": {\"version\": 1, \"license\": \"GPL-2.1-or-later\"},\n"
	        " \"layout\": {\"shrink\": false, \"widgets\": [{}]}}",
	.no_shrink = 1,
};

const struct tst_suite tst_suite = {
	.suite_name = "widget JSON testsuite",
	.tests = {
		{.name = "missing info",
		 .tst_fn = invalid_empty,
		 .data = &missing_info},

		{.name = "missing version",
		 .tst_fn = invalid_empty,
		 .data = &missing_version},

		{.name = "invalid version",
		 .tst_fn = invalid_empty,
		 .data = &invalid_version},

		{.name = "version not a number",
		 .tst_fn = invalid_empty,
		 .data = &version_not_number},

		{.name = "empty",
		 .tst_fn = invalid_empty,
		 .data = &empty},

		{.name = "not an object",
		 .tst_fn = invalid_empty,
		 .data = &not_object},

		{.name = "invalid widget type",
		 .tst_fn = invalid_empty,
		 .data = &invalid_widget_type},

		{.name = "widget type not a string",
		 .tst_fn = invalid_empty,
		 .data = &widget_type_not_string},

		{.name = "widget uid not a string",
		 .tst_fn = invalid_empty,
		 .data = &widget_uid_not_string},

		{.name = "widget default align",
		 .tst_fn = load_json,
		 .data = &widget_default_align},

		{.name = "widget align fill",
		 .tst_fn = load_json,
		 .data = &widget_align_fill},

		{.name = "widget align left",
		 .tst_fn = load_json,
		 .data = &widget_align_left},

		{.name = "widget align bottom",
		 .tst_fn = load_json,
		 .data = &widget_align_bottom},

		{.name = "widget uid",
		 .tst_fn = load_json,
		 .data = &widget_uid},

		{.name = "widget missing on_event",
		 .tst_fn = load_json,
		 .data = &widget_missing_on_event},

		{.name = "widget on_event",
		 .tst_fn = load_json,
		 .data = &widget_on_event},

		{.name = "widget no_shrink",
		 .tst_fn = load_json,
		 .data = &widget_no_shrink},

		{.name = NULL},
	}
};
