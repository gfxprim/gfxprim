// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <utils/gp_json.h>
#include "tst_test.h"

static void transform_obj(gp_json_reader *reader, gp_json_writer *writer, gp_json_val *val, const char *id);

static void transform_arr(gp_json_reader *reader, gp_json_writer *writer, gp_json_val *val, const char *id)
{
	gp_json_arr_start(writer, id);

	GP_JSON_ARR_FOREACH(reader, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			transform_obj(reader, writer, val, NULL);
		break;
		case GP_JSON_ARR:
			transform_arr(reader, writer, val, NULL);
		break;
		case GP_JSON_INT:
			gp_json_int_add(writer, NULL, val->val_int);
		break;
		case GP_JSON_FLOAT:
			gp_json_float_add(writer, NULL, val->val_float);
		break;
		case GP_JSON_BOOL:
			gp_json_bool_add(writer, NULL, val->val_bool);
		break;
		case GP_JSON_NULL:
			gp_json_null_add(writer, NULL);
		break;
		case GP_JSON_STR:
			gp_json_str_add(writer, NULL, val->val_str);
		break;
		case GP_JSON_VOID:
		break;
		}
	}

	gp_json_arr_finish(writer);
}

static void transform_obj(gp_json_reader *reader, gp_json_writer *writer, gp_json_val *val, const char *id)
{
	gp_json_obj_start(writer, id);

	GP_JSON_OBJ_FOREACH(reader, val) {
		switch (val->type) {
		case GP_JSON_OBJ:
			transform_obj(reader, writer, val, val->id);
		break;
		case GP_JSON_ARR:
			transform_arr(reader, writer, val, val->id);
		break;
		case GP_JSON_INT:
			gp_json_int_add(writer, val->id, val->val_int);
		break;
		case GP_JSON_FLOAT:
			gp_json_float_add(writer, val->id, val->val_float);
		break;
		case GP_JSON_BOOL:
			gp_json_bool_add(writer, val->id, val->val_bool);
		break;
		case GP_JSON_NULL:
			gp_json_null_add(writer, val->id);
		break;
		case GP_JSON_STR:
			gp_json_str_add(writer, val->id, val->val_str);
		break;
		case GP_JSON_VOID:
		break;
		}
	}

	gp_json_obj_finish(writer);
}

static void transform(gp_json_reader *reader, gp_json_writer *writer, gp_json_val *val)
{
	switch (gp_json_start(reader)) {
	case GP_JSON_OBJ:
		transform_obj(reader, writer, val, NULL);
	break;
	case GP_JSON_ARR:
		transform_arr(reader, writer, val, NULL);
	break;
	default:
	break;
	}

	gp_json_finish(writer);
}

static int json_read_write_test(const char *fname)
{
	gp_json_reader *reader = gp_json_reader_load(fname);
	gp_json_writer *writer = gp_json_writer_vec_new();
	char buf[128];
	gp_json_val val = {
		.buf = buf,
		.buf_size = sizeof(buf),
	};

	if (!reader) {
		tst_msg("Failed to load '%s'", fname);
		return TST_FAILED;
	}

	if (!writer) {
		tst_msg("Allocation failure");
		return TST_FAILED;
	}

	transform(reader, writer, &val);

	if (gp_json_reader_err(reader)) {
		tst_msg("Failure in reader!");
		gp_json_err_print(reader);
		return TST_FAILED;
	}

	if (gp_json_writer_err(writer)) {
		tst_msg("Failure in writer!");
		printf("%s\n", gp_json_writer_vec(writer));
		//TODO: Print err!
		return TST_FAILED;
	}

	if (strcmp(gp_json_writer_vec(writer), reader->buf)) {
		tst_msg("JSON is different");
		printf("'%s'\n", reader->buf);
		printf("'%s'\n", gp_json_writer_vec(writer));
		return TST_FAILED;
	}


	gp_json_reader_free(reader);
	gp_json_writer_vec_free(writer);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON read/write testsuite",
	.tests = {
		{.name = "arr empty",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_empty.json",
                 .res_path = "data/json/arr_empty.json"},

		{.name = "arr int",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_int.json",
                 .res_path = "data/json/arr_int.json"},

		{.name = "arr mixed",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_mixed.json",
                 .res_path = "data/json/arr_mixed.json"},

		{.name = "arr obj",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_obj.json",
                 .res_path = "data/json/arr_obj.json"},

		{.name = "arr str",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_str.json",
                 .res_path = "data/json/arr_str.json"},

		{.name = "arr bool",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_bool.json",
                 .res_path = "data/json/arr_bool.json"},

		{.name = "arr float",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"arr_float.json",
                 .res_path = "data/json/arr_float.json"},

		{.name = "str esc",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"str_esc.json",
                 .res_path = "data/json/str_esc.json"},

		{.name = "str utf",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"str_utf.json",
                 .res_path = "data/json/str_utf.json"},

		{.name = "id utf",
		 .tst_fn = json_read_write_test,
		 .flags = TST_TMPDIR | TST_CHECK_MALLOC,
		 .data = (void*)"id_utf.json",
                 .res_path = "data/json/id_utf.json"},

		{}
	}
};
