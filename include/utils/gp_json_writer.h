// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_json_writer.h
 * @brief A JSON writer.
 *
 * All the function that add values return zero on success and non-zero on a
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

/**
 * @brief A gp_json_writer initializer with default values.
 *
 * @param vout A pointer to function to write out the data.
 * @param vout_priv An user pointer passed to the out function.
 *
 * @return A gp_json_writer initialized with default values.
 */
#define GP_JSON_WRITER_INIT(vout, vout_priv) { \
	.err_print = GP_JSON_ERR_PRINT, \
	.err_print_priv = GP_JSON_ERR_PRINT_PRIV, \
	.out = vout, \
	.out_priv = vout_priv \
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
 * @param self A JSON writer.
 *
 * @return A pointer to the vector storage.
 */
static inline char *gp_json_writer_vec(gp_json_writer *self)
{
	return self->out_priv;
}

/**
 * @brief Allocates a JSON file writer.
 *
 * The call may fail either when file cannot be opened for writing or if
 * allocation has failed. In all cases errno should be set correctly.
 *
 * @param path A path to the file, file is opened for writing and created if it
 *             does not exist.
 *
 * @return A gp_json_writer pointer or NULL in a case of failure.
 */
gp_json_writer *gp_json_writer_file_open(const char *path);

/**
 * @brief Closes and frees a JSON file writer.
 *
 * @param self A gp_json_writer file writer.
 *
 * @return Zero on success, non-zero on a failure and errno is set.
 */
int gp_json_writer_file_close(gp_json_writer *self);

/**
 * @brief Returns true if writer error happened.
 *
 * @param self A JSON writer.
 *
 * @return True if error has happened.
 */
static inline int gp_json_writer_err(gp_json_writer *self)
{
	return !!self->err[0];
}

/**
 * @brief Starts a JSON object.
 *
 * For a top level object the id must be NULL, every other object has to have
 * non-NULL id. The call will also fail if maximal recursion depth
 * GP_JSON_RECURSION_MAX has been reached.
 *
 * @param self A JSON writer.
 * @param id An object name.
 *
 * @return Zero on a success, non-zero otherwise.
 */
int gp_json_obj_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes a JSON object.
 *
 * The call will fail if we are currenlty not writing out an object.
 *
 * @param self A JSON writer.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_finish(gp_json_writer *self);

/**
 * @brief Starts a JSON array.
 *
 * For a top level array the id must be NULL, every other array has to have
 * non-NULL id. The call will also fail if maximal recursion depth
 * GP_JSON_RECURSION_MAX has been reached.
 *
 * @param self A JSON writer.
 * @param id An array name.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes a JSON array.
 *
 * The call will fail if we are currenlty not writing out an array.
 *
 * @param self A JSON writer.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_finish(gp_json_writer *self);

/**
 * @brief Adds a null value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A null value name.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_null_add(gp_json_writer *self, const char *id);

/**
 * @brief Adds an integer value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id An integer value name.
 * @param val An integer value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_int_add(gp_json_writer *self, const char *id, long val);

/**
 * @brief Adds a bool value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id An boolean value name.
 * @param val A boolean value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_bool_add(gp_json_writer *self, const char *id, int val);

/**
 * @brief Adds a float value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A floating point value name.
 * @param val A floating point value.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_float_add(gp_json_writer *self, const char *id, double val);

/**
 * @brief Adds a string value.
 *
 * The id must be NULL inside of an array, and must be non-NULL inside of an
 * object.
 *
 * @param self A JSON writer.
 * @param id A string value name.
 * @param str A UTF8 string value.
 *
 * @return Zero on success, non-zero otherwise.
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
int gp_json_writer_finish(gp_json_writer *self);

#endif /* GP_JSON_WRITER_H */
