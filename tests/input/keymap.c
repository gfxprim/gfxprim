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
#include <input/gp_ev_queue.h>
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

static int expect_char(gp_ev_queue *queue, uint32_t key, uint32_t ch)
{
	gp_event *ev = gp_ev_queue_get(queue);
	if (!ev) {
		tst_msg("No event queued");
		return TST_FAILED;
	}

	if (ev->type != GP_EV_KEY) {
		tst_msg("Wrong event type");
		return TST_FAILED;
	}

	if (ev->key.key != key) {
		tst_msg("Wrong key pressed %u expected %u",
		        (unsigned int)ev->key.key, (unsigned int)key);
		return TST_FAILED;
	}

	if (ev->key.utf != ch) {
		tst_msg("Wrong character '%c' generated expected '%c'",
		        ev->key.utf, ch);
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int test_keymap_shift_caps(void)
{
	gp_keymap *keymap;
	gp_ev_queue queue;

	gp_ev_queue_init(&queue, 10, 10, GP_EVENT_QUEUE_SIZE, NULL, NULL, 0);

	keymap = gp_keymap_json_load(KEYMAP);
	if (!keymap) {
		tst_msg("Failed to load keymap!");
		return TST_FAILED;
	}

	queue.keymap = keymap;

	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_A, 'a'))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_UP, 0, 0);

	if (expect_char(&queue, GP_KEY_A, 0))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_LEFT_SHIFT, GP_EV_KEY_DOWN, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_B, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_LEFT_SHIFT, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_B, 'B'))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_CAPS_LOCK, GP_EV_KEY_DOWN, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_CAPS_LOCK, GP_EV_KEY_UP, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_CAPS_LOCK, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_CAPS_LOCK, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_A, 'a'))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_LEFT_SHIFT, GP_EV_KEY_UP, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_LEFT_SHIFT, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_A, 'A'))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_CAPS_LOCK, GP_EV_KEY_DOWN, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_CAPS_LOCK, GP_EV_KEY_UP, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_B, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_CAPS_LOCK, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_CAPS_LOCK, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_B, 'b'))
		return TST_FAILED;

	gp_keymap_free(keymap);

	return TST_PASSED;
}

static int test_keymap_dead(void)
{
	gp_keymap *keymap;
	gp_ev_queue queue;

	gp_ev_queue_init(&queue, 10, 10, GP_EVENT_QUEUE_SIZE, NULL, NULL, 0);

	keymap = gp_keymap_json_load(KEYMAP);
	if (!keymap) {
		tst_msg("Failed to load keymap!");
		return TST_FAILED;
	}

	queue.keymap = keymap;

	gp_ev_queue_push_key(&queue, GP_KEY_EQUAL, GP_EV_KEY_DOWN, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_EQUAL, GP_EV_KEY_UP, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, 0, 0);
	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_UP, 0, 0);

	if (expect_char(&queue, GP_KEY_EQUAL, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_EQUAL, 0))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_A, '*'))
		return TST_FAILED;

	if (expect_char(&queue, GP_KEY_A, 0))
		return TST_FAILED;

	gp_ev_queue_push_key(&queue, GP_KEY_A, GP_EV_KEY_DOWN, 0, 0);

	if (expect_char(&queue, GP_KEY_A, 'a'))
		return TST_FAILED;

	gp_keymap_free(keymap);

	return TST_PASSED;
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
