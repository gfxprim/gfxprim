// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <utils/gp_json.h>
#include "tst_test.h"

static void parse_obj(gp_json_reader *buf, gp_json_val *val);

static void parse_arr(gp_json_reader *buf, gp_json_val *val)
{
	GP_JSON_ARR_FOREACH(buf, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			parse_obj(buf, val);
		break;
		case GP_JSON_ARR:
			parse_arr(buf, val);
		break;
		default:
		break;
		}
	}
}

static void parse_obj(gp_json_reader *buf, gp_json_val *val)
{
	GP_JSON_OBJ_FOREACH(buf, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			parse_obj(buf, val);
		break;
		case GP_JSON_ARR:
			parse_arr(buf, val);
		break;
		default:
		break;
		}
	}
}

static void parse(gp_json_reader *buf, gp_json_val *val)
{
	switch (gp_json_start(buf)) {
	case GP_JSON_OBJ:
		parse_obj(buf, val);
	break;
	case GP_JSON_ARR:
		parse_arr(buf, val);
	break;
	default:
	break;
	}
}

#define JSON_REC_01 "{\"a\": {\"a\": {\"a\": {\"a\": {\"a\": 1}}}}}"
#define JSON_REC_02 "[[[[[null]]]]]"

static int json_recursion_limit(const char *str)
{
	gp_json_reader json = GP_JSON_READER_INIT(str, strlen(str));

	char buf[128];

	gp_json_val val = {
		.buf = buf,
		.buf_size = sizeof(buf),
	};

	json.max_depth = 4;

	parse(&json, &val);
	if (!gp_json_reader_err(&json)) {
		tst_msg("No error on reaching recursion limit");
		return TST_FAILED;
	}

	tst_msg("Got: '%s' %u", json.err, json.depth);

	gp_json_reset(&json);

	json.max_depth = 5;

	parse(&json, &val);
	if (gp_json_reader_err(&json)) {
		tst_msg("Recursion limit reached unexpectedly");
		tst_msg("Got: '%s'", json.err);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON parser testsuite",
	.tests = {
		{.name = "JSON recursion limit OBJ",
		 .tst_fn = json_recursion_limit,
		 .data = JSON_REC_01},

		{.name = "JSON recursion limit ARR",
		 .tst_fn = json_recursion_limit,
		 .data = JSON_REC_02},

		{}
	}
};
