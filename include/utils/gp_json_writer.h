// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_json_writer.h
 * @brief A JSON writer.
 *
 * All the function that add values return 0 on success and non-zero on a
 * failure. Once an error has happened all subsequent attempts to add more
 * values return with non-zero exit status immediatelly. This is designed
 * so that we can add several values without checking each return value
 * and only check if error has happened at the end of the sequence.
 *
 * Failures may occur:
 * - if we call the functions out of order, e.g. attempt to finish array when
 *   we are not writing out an array.
 * - if we run out of recursion stack
 * - may be propagated from the writer function, e.g. allocation failure, no
 *   space on disk, etc.
 */

#ifndef GP_JSON_WRITER_H
#define GP_JSON_WRITER_H

#include <utils/gp_json_common.h>

/** @brief A JSON writer */
struct gp_json_writer {
	unsigned int depth;
	char depth_type[GP_JSON_RECURSION_MAX/8];
	char depth_first[GP_JSON_RECURSION_MAX/8];

	/** Handler to print errors and warnings */
	void (*err_print)(void *err_print_priv, const char *line);
	void *err_print_priv;
	char err[GP_JSON_ERR_MAX];

	/** Handler to produce JSON output */
	int (*out)(struct gp_json_writer *self, const char *buf, size_t buf_size);
	void *out_priv;
};

#define GP_JSON_WRITER_INIT(out_, out_priv_) { \
	.err_print = GP_JSON_ERR_PRINT, \
	.err_print_priv = GP_JSON_ERR_PRINT_PRIV, \
	.out = out_, \
	.out_priv = out_priv_ \
}

/**
 * @brief Allocates a JSON writer with a vector as a storage.
 *
 * @return A gp_json_writer pointer or NULL in a case of allocation failure.
 */
gp_json_writer *gp_json_writer_vec_new(void);

/**
 * @brief Frees a JSON writer with a vector as a storage.
 *
 * @param self A gp_json_writer with vector as a storage.
 */
void gp_json_writer_vec_free(gp_json_writer *self);

/**
 * @brief Returns a pointer to a vector storage.
 *
 * @return A pointer to the vector storage.
 */
static inline char *gp_json_writer_vec(gp_json_writer *self)
{
	return self->out_priv;
}

/**
 * @brief Returns true if writer error happened.
 *
 * @return True if error has happened.
 */
static inline int gp_json_writer_err(gp_json_writer *self)
{
	return !!self->err[0];
}

/**
 * @brief Starts an JSON object.
 *
 * @param self A JSON writer.
 * @param id An object name.
 */
int gp_json_obj_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes a JSON object.
 *
 * @param self A JSON writer.
 */
int gp_json_obj_finish(gp_json_writer *self);

/**
 * @brief Starts an json array.
 *
 * @param self A JSON writer.
 * @param id An array name.
 */
int gp_json_arr_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes a JSON array.
 *
 * @param self A JSON writer.
 */
int gp_json_arr_finish(gp_json_writer *self);

/**
 * @brief Adds a null value.
 *
 * @param self A JSON writer.
 * @param id A null value name.
 */
int gp_json_null_add(gp_json_writer *self, const char *id);

/**
 * @brief Adds an integer value.
 *
 * @param self A JSON writer.
 * @param id An integer value name.
 */
int gp_json_int_add(gp_json_writer *self, const char *id, long val);

/**
 * @brief Adds a bool value.
 *
 * @param self A JSON writer.
 * @param id An boolean value name.
 */
int gp_json_bool_add(gp_json_writer *self, const char *id, int val);

/**
 * @brief Adds a float value.
 *
 * @param self A JSON writer.
 * @param id A floating point value name.
 */
int gp_json_float_add(gp_json_writer *self, const char *id, double val);

/**
 * @brief Adds a string value.
 *
 * @param self A JSON writer.
 * @param id A string value name.
 */
int gp_json_str_add(gp_json_writer *self, const char *id, const char *str);

/**
 * @brief Finalizes json writer.
 *
 * Finalizes the json writer, throws possible errors through the error printing
 * function.
 *
 * @param self A JSON writer.
 * @return Overall error value.
 */
int gp_json_finish(gp_json_writer *self);

#endif /* GP_JSON_WRITER_H */
