// SPDX-License-Identifier: GPL-2.1-or-later
/*

  Copyright (C) 2007-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <utils/gp_json.h>
#include <utils/gp_vec.h>
#include "tst_test.h"

struct test_struct {
	char *str1;
	char str2[10];
	int val;
};

static gp_json_struct test_struct_desc[] = {
	GP_JSON_SERDES_STR_DUP(struct test_struct, str1, 1024),
	GP_JSON_SERDES_STR_CPY(struct test_struct, str2, 10),
	GP_JSON_SERDES_INT(struct test_struct, val, -100, 100),
	{}
};

static int json_write_read_struct(void)
{
	gp_json_writer *writer = gp_json_writer_vec_new();
	struct test_struct ser = {.str1 = "hello", .str2 = "world", .val = 42};
	struct test_struct des = {};
	int ret;

	if (!writer) {
		tst_msg("Failed to allocate writer");
		return TST_FAILED;
	}

	ret = gp_json_write_struct(writer, test_struct_desc, NULL, &ser);
	if (ret) {
		tst_msg("Failed to write struct");
		return TST_FAILED;
	}

	char *vec = gp_json_writer_vec(writer);

	gp_json_reader reader = GP_JSON_READER_INIT(vec, gp_vec_len(vec));

	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};

	ret = gp_json_read_struct(&reader, &val, test_struct_desc, &des);
	if (ret) {
		tst_msg("Failed to read struct");
		return TST_FAILED;
	}

	//tst_msg("%s", vec);

	gp_json_writer_vec_free(writer);

	if (strcmp(ser.str1, des.str1)) {
		tst_msg("Wrong str dup");
		return TST_FAILED;
	}

	tst_msg("str1 read as %s", des.str1);

	if (strcmp(ser.str2, des.str2)) {
		tst_msg("Wrong str cpy");
		return TST_FAILED;
	}

	tst_msg("str2 read as %s", des.str2);

	if (ser.val != des.val) {
		tst_msg("Wrong int value %i expected %i", des.val, ser.val);
		return TST_FAILED;
	}

	tst_msg("val read as %i", des.val);

	return TST_SUCCESS;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON serdes testsuite",
	.tests = {
		{.name = "JSON write read struct",
		 .tst_fn = json_write_read_struct},

		{}
	}
};
