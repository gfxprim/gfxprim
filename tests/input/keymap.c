// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Test for keymaps.

 */

#include <stdlib.h>
#include <unistd.h>
#include <input/gp_event.h>
#include <input/gp_event_queue.h>
#include <input/gp_keymap.h>

#include "tst_test.h"

#define KEYMAP "{\n"\
	       " \"mods\": {\n"\
	       "  \"mod\": \"KeyLeftShift\",\n"\
	       "  \"mod\": \"KeyRightShift\",\n"\
	       "  \"lock\": \"KeyCapsLock\",\n"\
	       "  \"dead\": \"KeyEqual\"\n"\
	       " },\n"\
	       "\"map\": {\n"\
	       "  \"mods\": [\n"\
	       "   [],\n"\
	       "   [\"KeyCapsLock\", \"KeyLeftShift\"],\n"\
	       "   [\"KeyCapsLock\", \"KeyRightShift\"],\n"\
	       "   [\"KeyCapsLock\", \"KeyRightShift\", \"KeyLeftShift\"]\n"\
	       "  ],\n"\
	       "  \"keys\": [\n"\
	       "    {\"KeyA\": \"a\"},\n"\
	       "    {\"KeyB\": \"b\"}\n"\
	       "  ]\n"\
	       " },\n"\
	       "\"map\": {\n"\
	       "  \"mods\": [\n"\
	       "   [\"KeyEqual\"],\n"\
	       "   [\"KeyEqual\", \"KeyRightShift\", \"KeyCapsLock\"],\n"\
	       "   [\"KeyEqual\", \"KeyLeftShift\", \"KeyCapsLock\"],\n"\
	       "   [\"KeyEqual\", \"KeyRightShift\", \"KeyLeftShift\", \"KeyCapsLock\"]\n"\
	       "  ],\n"\
	       "  \"keys\": [\n"\
	       "    {\"KeyA\": \"*\"}\n"\
	       "  ]\n"\
	       " },\n"\
	       "\"map\": {\n"\
	       "  \"mods\": [\n"\
	       "   [\"KeyCapsLock\"],\n"\
	       "   [\"KeyRightShift\"],\n"\
	       "   [\"KeyLeftShift\"],\n"\
	       "   [\"KeyRightShift\", \"KeyLeftShift\"]\n"\
	       "  ],\n"\
	       "  \"keys\": [\n"\
	       "    {\"KeyA\": \"A\"},\n"\
	       "    {\"KeyB\": \"B\"}\n"\
	       "  ]\n"\
	       " }\n"\
	       "}\n"

static int expect_char(gp_event_queue *queue, uint32_t ch)
{
	gp_event *ev = gp_event_queue_get(queue);
	if (!ev) {
		tst_msg("No event queued");
		return TST_FAILED;
	}

	if (ev->type != GP_EV_UTF) {
		tst_msg("Wrong event type");
		return TST_FAILED;
	}

	if (ev->utf.ch != ch) {
		tst_msg("Wrong character '%c' generated expected '%c'",
		        ev->utf.ch, ch);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

static int test_keymap_shift_caps(void)
{
	gp_keymap *keymap;
	gp_event_queue queue;

	gp_event_queue_init(&queue, 10, 10, GP_EVENT_QUEUE_SIZE, 0);

	keymap = gp_keymap_json_load(KEYMAP);
	if (!keymap) {
		tst_msg("Failed to load keymap!");
		return TST_FAILED;
	}

	gp_event key = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_DOWN,
		.val = GP_KEY_A,
	};

	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'a'))
		return TST_FAILED;

	key.val = GP_KEY_LEFT_SHIFT;
	gp_keymap_event_key(keymap, &queue, &key);

	key.val = GP_KEY_B;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'B'))
		return TST_FAILED;

	key.val = GP_KEY_CAPS_LOCK;
	gp_keymap_event_key(keymap, &queue, &key);

	key.val = GP_KEY_A;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'a'))
		return TST_FAILED;

	key.val = GP_KEY_LEFT_SHIFT;
	key.code = GP_EV_KEY_UP;
	gp_keymap_event_key(keymap, &queue, &key);

	key.val = GP_KEY_A;
	key.code = GP_EV_KEY_DOWN;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'A'))
		return TST_FAILED;

	key.val = GP_KEY_CAPS_LOCK;
	gp_keymap_event_key(keymap, &queue, &key);

	key.val = GP_KEY_B;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'b'))
		return TST_FAILED;

	gp_keymap_free(keymap);

	return TST_SUCCESS;
}

static int test_keymap_dead(void)
{
	gp_keymap *keymap;
	gp_event_queue queue;

	gp_event_queue_init(&queue, 10, 10, GP_EVENT_QUEUE_SIZE, 0);

	keymap = gp_keymap_json_load(KEYMAP);
	if (!keymap) {
		tst_msg("Failed to load keymap!");
		return TST_FAILED;
	}

	gp_event key = {
		.type = GP_EV_KEY,
		.code = GP_EV_KEY_DOWN,
		.val = GP_KEY_EQUAL,
	};

	gp_keymap_event_key(keymap, &queue, &key);

	key.val = GP_KEY_A;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, '*'))
		return TST_FAILED;

	key.val = GP_KEY_A;
	gp_keymap_event_key(keymap, &queue, &key);

	if (expect_char(&queue, 'a'))
		return TST_FAILED;

	gp_keymap_free(keymap);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "Key Map Testsuite",
	.tests = {
		{.name = "keymap shift + caps",
		 .tst_fn = test_keymap_shift_caps,
		 .flags = TST_CHECK_MALLOC},

		{.name = "keymap dead key",
		 .tst_fn = test_keymap_dead,
		 .flags = TST_CHECK_MALLOC},

		{},
	}
};
