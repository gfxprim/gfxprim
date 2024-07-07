// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <utils/gp_json_writer.h>

#define FILENAME "out.json"

int main(void)
{
	gp_json_writer *writer = gp_json_writer_file_open(FILENAME);

	if (!writer) {
		fprintf(stderr, "Failed to open '%s': %s\n", FILENAME, strerror(errno));
		return 1;
	}

	gp_json_obj_start(writer, NULL);
	gp_json_obj_start(writer, "object");
	gp_json_int_add(writer, "int_val01", 0);
	gp_json_int_add(writer, "int_int02", 100);
	gp_json_null_add(writer, "no_value");
	gp_json_obj_finish(writer);
	gp_json_obj_start(writer, "bar");
	gp_json_obj_finish(writer);
	gp_json_obj_start(writer, "buzz");
	gp_json_obj_finish(writer);
	gp_json_arr_start(writer, "array");
	gp_json_int_add(writer, NULL, 0);
	gp_json_int_add(writer, NULL, 1);
	gp_json_int_add(writer, NULL, 2);
	gp_json_arr_finish(writer);
	gp_json_obj_finish(writer);

	if (gp_json_writer_file_close(writer)) {
		fprintf(stderr, "Failed to close '%s': %s\n", FILENAME, strerror(errno));
		return 1;
	}

	fprintf(stderr, "JSON written into '%s'\n", FILENAME);

	return 0;
}
