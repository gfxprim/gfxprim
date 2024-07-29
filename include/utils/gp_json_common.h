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

/** @brief Maximal error message length. */
#define GP_JSON_ERR_MAX 128
/** @brief Maximal id string lenght including terminating null element. */
#define GP_JSON_ID_MAX 64
/** @brief Maximal recursion depth allowed. */
#define GP_JSON_RECURSION_MAX 128

#define GP_JSON_ERR_PRINT gp_json_err_handler
#define GP_JSON_ERR_PRINT_PRIV stderr

/**
 * @brief A JSON data type.
 */
enum gp_json_type {
	/** @brief No type. Returned when parser finishes. */
	GP_JSON_VOID = 0,
	/** @brief An integer. */
	GP_JSON_INT,
	/** @brief A floating point. */
	GP_JSON_FLOAT,
	/** @brief A boolean. */
	GP_JSON_BOOL,
	/** @brief NULL */
	GP_JSON_NULL,
	/** @brief A string. */
	GP_JSON_STR,
	/** @brief A JSON object. */
	GP_JSON_OBJ,
	/** @brief A JSON array. */
	GP_JSON_ARR,
};

/**
 * @brief Returns type name.
 *
 * @param type A json type.
 * @return A type name.
 */
const char *gp_json_type_name(enum gp_json_type type);

/**
 * @brief Default error print handler.
 *
 * @param print_priv A json buffer print_priv pointer.
 * @param line A line to be printed.
 */
void gp_json_err_handler(void *print_priv, const char *line);

typedef struct gp_json_reader gp_json_reader;
typedef struct gp_json_writer gp_json_writer;
typedef struct gp_json_val gp_json_val;

#endif /* GP_JSON_COMMON_H */
