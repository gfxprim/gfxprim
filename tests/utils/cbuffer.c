// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <utils/gp_cbuffer.h>

#include "tst_test.h"

int cbuffer_test_empty(void)
{
	int cnt = 0;
	gp_cbuffer cbuf;
	gp_cbuffer_iter iter;
	size_t used;

	gp_cbuffer_init(&cbuf, 10);

	used = gp_cbuffer_used(&cbuf);
	if (used) {
		tst_msg("Empty buffer used %zu", used);
		return TST_FAILED;
	}

	GP_CBUFFER_FOREACH(&cbuf, &iter)
		cnt++;

	if (cnt || iter.cnt) {
		tst_msg("FOREACH() on empty list iterated over %i items cnt=%zu", cnt, iter.cnt);
		return TST_FAILED;
	}

	GP_CBUFFER_FOREACH_REV(&cbuf, &iter)
		cnt++;

	if (cnt || iter.cnt) {
		tst_msg("FOREACH_REV() on empty list iterated over %i items cnt=%zu", cnt, iter.cnt);
		return TST_FAILED;
	}

	return TST_PASSED;
}

int cbuffer_test_append(void)
{
	unsigned int i, failed = 0;
	gp_cbuffer cbuf;
	gp_cbuffer_iter iter;
	size_t idx;

	gp_cbuffer_init(&cbuf, 10);

	for (i = 0; i < 9; i++) {
		idx = gp_cbuffer_append(&cbuf);

		if (idx != i) {
			tst_msg("Append returned wrong index %zu expected %i",
			         idx, i);
			failed++;
		}

		if (gp_cbuffer_used(&cbuf) != i+1) {
			tst_msg("Wrong number of used elements %zu expected %u",
			        gp_cbuffer_used(&cbuf), i+1);
			failed++;
		}
	}

	tst_msg("After 9 appends first=%zu last=%zu",
	        gp_cbuffer_first(&cbuf), gp_cbuffer_last(&cbuf));

	tst_msg("Testing FOREACH() with cnt=9");

	i = 0;
	GP_CBUFFER_FOREACH(&cbuf, &iter) {
		i++;
	}

	if (i != 9) {
		tst_msg("FOREACH() did %i iterations, expected 9", i);
		failed++;
	}

	idx = gp_cbuffer_append(&cbuf);
	if (idx != 9) {
		tst_msg("append() returned %zu expected 9", idx);
		failed++;
	}

	tst_msg("After 10 appends first=%zu last=%zu",
	        gp_cbuffer_first(&cbuf), gp_cbuffer_last(&cbuf));

	tst_msg("Testing FOREACH() with cnt=10");

	i = 0;
	GP_CBUFFER_FOREACH(&cbuf, &iter) {
		if (iter.cnt != iter.idx) {
			tst_msg("FOREACH() wrong idx %zu expected %zu", iter.idx, iter.cnt);
			failed++;
		}
		i++;
	}

	if (i != 10) {
		tst_msg("FOREACH() did %i iterations, expected 10", i);
		failed++;
	}

	tst_msg("Testing FOREACH_REV() with cnt=10");

	i = 0;
	GP_CBUFFER_FOREACH_REV(&cbuf, &iter) {
		if (9 - iter.cnt != iter.idx) {
			tst_msg("FOREACH_REV() wrong idx %zu expected %zu", iter.idx, 9 - iter.cnt);
			failed++;
		}
		i++;
	}

	if (i != 10) {
		tst_msg("FOREACH_REV() did %i iterations, expected 10", i);
		failed++;
	}

	idx = gp_cbuffer_append(&cbuf);
	if (idx != 0) {
		tst_msg("append() returned %zu expected 0", idx);
		failed++;
	}

	tst_msg("After 11 appends first=%zu last=%zu",
	        gp_cbuffer_first(&cbuf), gp_cbuffer_last(&cbuf));

	tst_msg("Testing foreach with first=1");

	i = 0;
	GP_CBUFFER_FOREACH(&cbuf, &iter) {
		if (iter.idx != (i+1) % 10) {
			tst_msg("FOREACH() wrong idx %zu expected %u", iter.idx, (i+1) % 10);
			failed++;
		}
		i++;
	}

	if (i != 10) {
		tst_msg("FOREACH() did %i iterations, expected 10", i);
		failed++;
	}

	if (failed)
		return TST_FAILED;

	return TST_PASSED;
}

static int cbuffer_test_first(void)
{
	gp_cbuffer cbuf;
	size_t i, idx;
	int failed = 0;

	tst_msg("Testing with last = 0");

	gp_cbuffer_init(&cbuf, 3);
	cbuf.last = 0;

	for (i = 0; i < 100; i++) {
		idx = gp_cbuffer_first(&cbuf);

		if ((i < 3 && idx != 0) ||
		    (i > 3 && idx != i%3)) {
			tst_msg("Wrong first index %zu at %zu", idx, i);
			failed++;
		}
		gp_cbuffer_append(&cbuf);
	}

	tst_msg("Testing with last = 1");

	gp_cbuffer_init(&cbuf, 3);
	cbuf.last = 1;

	for (i = 0; i < 100; i++) {
		idx = gp_cbuffer_first(&cbuf);

		if ((i < 3 && idx != 1) ||
		    (i > 3 && idx != (i+1)%3)) {
			tst_msg("Wrong first index %zu at %zu", idx, i);
			failed++;
		}
		gp_cbuffer_append(&cbuf);
	}

	if (failed)
		return TST_FAILED;

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "cbuffer testsuite",
	.tests = {
		{.name = "cbuffer test empty",
		 .tst_fn = cbuffer_test_empty},

		{.name = "cbuffer test append",
		 .tst_fn = cbuffer_test_append},

		{.name = "cbuffer test first",
		 .tst_fn = cbuffer_test_first},

		{}
	}
};
