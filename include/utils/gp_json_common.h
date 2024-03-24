// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_json_common.h
 * @brief Common JSON reader/writer definitions.
 */

#ifndef GP_JSON_COMMON_H
#define GP_JSON_COMMON_H

#define GP_JSON_ERR_MAX 128
#define GP_JSON_ID_MAX 64

#define GP_JSON_RECURSION_MAX 128

/**
 * @brief A JSON data type.
 */
enum gp_json_type {
	GP_JSON_VOID = 0,
	GP_JSON_INT,
	GP_JSON_FLOAT,
	GP_JSON_BOOL,
	GP_JSON_NULL,
	GP_JSON_STR,
	GP_JSON_OBJ,
	GP_JSON_ARR,
};

#define GP_JSON_ERR_PRINT gp_json_err_handler
#define GP_JSON_ERR_PRINT_PRIV stderr

/**
 * @brief default error print handler
 *
 * @print_priv A json buffer print_priv pointer.
 * @line A line of output to be printed.
 */
void gp_json_err_handler(void *print_priv, const char *line);

typedef struct gp_json_reader gp_json_reader;
typedef struct gp_json_writer gp_json_writer;

#endif /* GP_JSON_COMMON_H */
