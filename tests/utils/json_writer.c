// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <utils/gp_json.h>
#include "tst_test.h"

static int out(gp_json_writer *self, const char *str, size_t len)
{
	(void) self;
	(void) str;
	(void) len;

	return 0;
}

static int json_finalize_err_obj(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	if (gp_json_obj_start(&writer, NULL)) {
		tst_msg("Failed to start object");
		return TST_FAILED;
	}

	if (!gp_json_finish(&writer)) {
		tst_msg("Finish didn't fail with unfinished object");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int json_finalize_err_arr(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	if (gp_json_arr_start(&writer, NULL)) {
		tst_msg("Failed to start array");
		return TST_FAILED;
	}

	if (!gp_json_finish(&writer)) {
		tst_msg("Finish didn't fail with unfinished object");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int json_obj_add_without_id(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	if (gp_json_obj_start(&writer, NULL)) {
		tst_msg("Failed to start object");
		return TST_FAILED;
	}

	if (!gp_json_int_add(&writer, NULL, 0)) {
		tst_msg("Add to object without id didn't fail!");
		return TST_FAILED;
	}

	if (!writer.err[0]) {
		tst_msg("Error message is not set!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int json_arr_add_with_id(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	if (gp_json_arr_start(&writer, NULL)) {
		tst_msg("Failed to start array");
		return TST_FAILED;
	}

	if (!gp_json_int_add(&writer, "id", 0)) {
		tst_msg("Add to array with id didn't fail!");
		return TST_FAILED;
	}

	if (!writer.err[0]) {
		tst_msg("Error message is not set!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

static int json_add_without_obj_arr(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	if (!gp_json_int_add(&writer, "id", 0)) {
		tst_msg("Add without obj/arr start didn't fail!");
		return TST_FAILED;
	}

	if (!writer.err[0]) {
		tst_msg("Error message is not set!");
		return TST_FAILED;
	}

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON writer testsuite",
	.tests = {
		{.name = "JSON finalize err obj",
		 .tst_fn = json_finalize_err_obj},

		{.name = "JSON finalize err arr",
		 .tst_fn = json_finalize_err_arr},

		{.name = "JSON obj add without id",
		 .tst_fn = json_obj_add_without_id},

		{.name = "JSON arr add with id",
		 .tst_fn = json_arr_add_with_id},

		{.name = "JSON add without obj/arr start",
		 .tst_fn = json_add_without_obj_arr},

		{}
	}
};
