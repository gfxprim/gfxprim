// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/gp_json_writer.h>

static int out(gp_json_writer *self, const char *buf, size_t buf_len)
{
	size_t i;

	(void) self;

	for (i = 0; i < buf_len; i++)
		fputc(buf[i], stdout);

	return 0;
}

int main(void)
{
	gp_json_writer writer = GP_JSON_WRITER_INIT(out, NULL);

	gp_json_obj_start(&writer, NULL);
	gp_json_obj_start(&writer, "object");
	gp_json_int_add(&writer, "int_val01", 0);
	gp_json_int_add(&writer, "int_int02", 100);
	gp_json_null_add(&writer, "no_value");
	gp_json_obj_finish(&writer);
	gp_json_obj_start(&writer, "bar");
	gp_json_obj_finish(&writer);
	gp_json_obj_start(&writer, "buzz");
	gp_json_obj_finish(&writer);
	gp_json_arr_start(&writer, "array");
	gp_json_int_add(&writer, NULL, 0);
	gp_json_int_add(&writer, NULL, 1);
	gp_json_int_add(&writer, NULL, 2);
	gp_json_arr_finish(&writer);
	gp_json_obj_finish(&writer);

	return 0;
}
