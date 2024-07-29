// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <utils/gp_json_common.h>

void gp_json_err_handler(void *err_print_priv, const char *line)
{
	fputs(line, err_print_priv);
	putc('\n', err_print_priv);
}

const char *gp_json_type_name(enum gp_json_type type)
{
	switch (type) {
	case GP_JSON_VOID:
		return "void";
	case GP_JSON_INT:
		return "integer";
	case GP_JSON_FLOAT:
		return "float";
	case GP_JSON_BOOL:
		return "boolean";
	case GP_JSON_NULL:
		return "null";
	case GP_JSON_STR:
		return "string";
	case GP_JSON_OBJ:
		return "object";
	case GP_JSON_ARR:
		return "array";
	default:
		return "invalid";
	}
}
