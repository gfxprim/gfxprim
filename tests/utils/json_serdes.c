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
	uint64_t u64;
	uint32_t u32;
	uint16_t u16;
	uint8_t u8;
	int64_t i64;
	int32_t i32;
	int16_t i16;
	int8_t i8;
};

static gp_json_struct test_struct_desc[] = {
	GP_JSON_SERDES_STR_DUP(struct test_struct, str1, GP_JSON_SERDES_OPTIONAL, 1024, "greeting"),
	GP_JSON_SERDES_INT16(struct test_struct, i16, 0, -100, 100),
	GP_JSON_SERDES_INT32(struct test_struct, i32, 0, -100, 100),
	GP_JSON_SERDES_INT64(struct test_struct, i64, 0, -100, 100),
	GP_JSON_SERDES_INT8(struct test_struct, i8, 0, -100, 100),
	GP_JSON_SERDES_STR_CPY(struct test_struct, str2, 0, 10),
	GP_JSON_SERDES_UINT16(struct test_struct, u16, 0, -100, 100),
	GP_JSON_SERDES_UINT32(struct test_struct, u32, 0, -100, 100),
	GP_JSON_SERDES_UINT64(struct test_struct, u64, 0, -100, 100),
	GP_JSON_SERDES_UINT8(struct test_struct, u8, 0, -100, 100),
	GP_JSON_SERDES_INT(struct test_struct, val, 0, -100, 100),
	{}
};

#define COMPARE_INT(val1, val2, name) \
	if (val1 != val2) { \
		tst_msg("Wrong " name "int value %lli expected %lli", \
			(long long int)val1, (long long int)val2); \
		return TST_FAILED; \
	} \
	tst_msg("Value " name " read back as %lli", (long long int) val2);

static int json_write_read_struct(void)
{
	gp_json_writer *writer = gp_json_writer_vec_new();
	struct test_struct ser = {
		.str1 = "hello",
		.str2 = "world",
		.val = 42,
		.u64 = 64,
		.i64 = -64,
		.u32 = 32,
		.i32 = -32,
		.u16 = 16,
		.i16 = -16,
		.u8 = 8,
		.i8 = -8,
	};
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

//	tst_msg("%s", vec);

	gp_json_reader reader = GP_JSON_READER_INIT(vec, gp_vec_len(vec));

	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};

	ret = gp_json_read_struct(&reader, &val, test_struct_desc, &des);
	if (ret) {
		tst_msg("Failed to read struct");
		return TST_FAILED;
	}

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

	COMPARE_INT(ser.val, des.val, "val");
	COMPARE_INT(ser.i8, des.i8, "i8");
	COMPARE_INT(ser.u8, des.u8, "u8");
	COMPARE_INT(ser.i16, des.i16, "i16");
	COMPARE_INT(ser.u16, des.u16, "u16");
	COMPARE_INT(ser.i32, des.i32, "i32");
	COMPARE_INT(ser.u32, des.u32, "u32");
	COMPARE_INT(ser.i64, des.i64, "i64");
	COMPARE_INT(ser.u64, des.u64, "u64");

	return TST_PASSED;
}


static gp_json_struct opt_i16_desc[] = {
	GP_JSON_SERDES_INT16(struct test_struct, i16, GP_JSON_SERDES_OPTIONAL, -100, 100),
	GP_JSON_SERDES_INT32(struct test_struct, i32, 0, -100, 100),
	{}
};

#define JSON_MISSING_INT32 "{\"i16\": 42}"

static int json_read_missing_struct(void)
{
	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};
	struct test_struct des = {};
	gp_json_reader reader = GP_JSON_READER_INIT(JSON_MISSING_INT32,
	                                            sizeof(JSON_MISSING_INT32));

	int ret = gp_json_read_struct(&reader, &val, opt_i16_desc, &des);
	if (ret) {
		tst_msg("Failed to read strucure as expected");
		return TST_PASSED;
	}

	return TST_FAILED;
}

#define JSON_MISSING_INT16 "{\"i32\": 42}"

static int json_read_opt_struct(void)
{
	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};
	struct test_struct des = {};
	gp_json_reader reader = GP_JSON_READER_INIT(JSON_MISSING_INT16,
	                                            sizeof(JSON_MISSING_INT16));

	int ret = gp_json_read_struct(&reader, &val, opt_i16_desc, &des);
	if (ret) {
		tst_msg("Failed to read strucure");
		return TST_FAILED;
	}

	COMPARE_INT(42, des.i32, "i32");

	return TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "JSON serdes testsuite",
	.tests = {
		{.name = "JSON write read struct",
		 .tst_fn = json_write_read_struct},

		{.name = "JSON read missing struct",
		 .tst_fn = json_read_missing_struct},

		{.name = "JSON read opt struct",
		 .tst_fn = json_read_opt_struct},

		{}
	}
};
