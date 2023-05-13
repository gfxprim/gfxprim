// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_trie.h>

#include "tst_test.h"

static int trie_lookup(void)
{
	gp_trie_node *root = NULL;
	gp_trie_node *ret;
	int fail = 0;

	root = gp_trie_ins(root, "test", (void*)1l);
	root = gp_trie_ins(root, "te", (void*)2l);
	root = gp_trie_ins(root, "hello", (void*)3l);
	root = gp_trie_ins(root, "hell", (void*)4l);

	ret = gp_trie_lookup(root, "t");
	if (!ret) {
		tst_msg("Lookup for 't' failed");
		fail++;
	} else if (gp_trie_payload(ret)) {
		tst_msg("Got payload for 't'");
		fail++;
	}

	ret = gp_trie_lookup(root, "tes");
	if (!ret) {
		tst_msg("Lookup for 'tes' failed");
		fail++;
	} else if (gp_trie_payload(ret)) {
		tst_msg("Got payload for 'tes'");
		fail++;
	}

	if (gp_trie_lookup(root, "testy")) {
		tst_msg("Got lookup result for 'testy'");
		fail++;
	}

	ret = gp_trie_lookup(root, "te");
	if (!ret) {
		tst_msg("No lookup result for 'te'");
		fail++;
	} else if (gp_trie_payload(ret) != (void*)2l) {
		tst_msg("Wrong payload for 'te'");
		fail++;
	}

	ret = gp_trie_lookup(root, "te");
	if (!ret) {
		tst_msg("No lookup result for 'te'");
		fail++;
	} else if (gp_trie_payload(ret) != (void*)2l) {
		tst_msg("Wrong payload for 'te'");
		fail++;
	}

	gp_trie_free(root, NULL);

	return TST_PASSED;
}

static int trie_inner_del(void)
{
	gp_trie_node *root = NULL;
	gp_trie_node *ret;
	void *payload = NULL;

	root = gp_trie_ins(root, "test", (void*)1l);
	root = gp_trie_ins(root, "te", (void*)2l);

	root = gp_trie_del(root, "te", &payload);
	if (payload != (void*)2l) {
		tst_msg("Wrong payload for deleted 'te' key %p", payload);
		return TST_FAILED;
	}

	ret = gp_trie_lookup(root, "te");
	if (gp_trie_payload(ret)) {
		tst_msg("Got lookup for deleted 'te' key");
		return TST_FAILED;
	}

	gp_trie_free(root, NULL);

	return TST_PASSED;
}

static int trie_outer_del(void)
{
	gp_trie_node *root = NULL;
	gp_trie_node *ret;
	void *payload = NULL;

	root = gp_trie_ins(root, "test", (void*)1l);
	root = gp_trie_ins(root, "te", (void*)2l);

	root = gp_trie_del(root, "test", &payload);
	if (payload != (void*)1l) {
		tst_msg("Wrong payload for deleted 'test' key %p", payload);
		return TST_FAILED;
	}

	ret = gp_trie_lookup(root, "test");
	if (ret) {
		tst_msg("Got lookup for deleted 'test' key");
		return TST_FAILED;
	}

	ret = gp_trie_lookup(root, "tes");
	if (ret) {
		tst_msg("Non-leaf node 'tes' not deleted!");
		return TST_FAILED;
	}

	gp_trie_free(root, NULL);

	return TST_PASSED;
}

static int trie_nonexist_del(void)
{
	gp_trie_node *root = NULL;
	void *payload;

	root = gp_trie_ins(root, "test", (void*)1l);
	root = gp_trie_ins(root, "te", (void*)2l);

	root = gp_trie_del(root, "nonexist", &payload);

	gp_trie_free(root, NULL);

	return TST_PASSED;
}

static int trie_del_all(void)
{
	gp_trie_node *root = NULL;
	void *payload = NULL;

	root = gp_trie_ins(root, "test", (void*)1l);
	root = gp_trie_ins(root, "te", (void*)2l);
	root = gp_trie_ins(root, "hello", (void*)3l);
	root = gp_trie_ins(root, "hell", (void*)4l);

	root = gp_trie_del(root, "test", &payload);
	if (payload != (void*)1l) {
		tst_msg("Wrong payload for deleted 'test' key %p", payload);
		return TST_FAILED;
	}

	root = gp_trie_del(root, "te", &payload);
	if (payload != (void*)2l) {
		tst_msg("Wrong payload for deleted 'te' key %p", payload);
		return TST_FAILED;
	}

	root = gp_trie_del(root, "hello", &payload);
	if (payload != (void*)3l) {
		tst_msg("Wrong payload for deleted 'te' key %p", payload);
		return TST_FAILED;
	}

	root = gp_trie_del(root, "hell", &payload);
	if (payload != (void*)4l) {
		tst_msg("Wrong payload for deleted 'te' key %p", payload);
		return TST_FAILED;
	}

	if (root) {
		tst_msg("Non NULL root for empty trie");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "trie testsuite",
	.tests = {
		{.name = "trie lookup",
		 .tst_fn = trie_lookup,
		 .flags = TST_CHECK_MALLOC},

		{.name = "trie inner del",
		 .tst_fn = trie_inner_del,
		 .flags = TST_CHECK_MALLOC},

		{.name = "trie outer del",
		 .tst_fn = trie_outer_del,
		 .flags = TST_CHECK_MALLOC},

		{.name = "trie nonexist del",
		 .tst_fn = trie_nonexist_del,
		 .flags = TST_CHECK_MALLOC},

		{.name = "trie del all",
		 .tst_fn = trie_del_all,
		 .flags = TST_CHECK_MALLOC},

		{}
	}
};
