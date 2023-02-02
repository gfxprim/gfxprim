// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_JSON_WRITER_H
#define GP_JSON_WRITER_H

#include <utils/gp_json_common.h>

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
 * @self A gp_json_writer with vector as a storage.
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
 */
static inline int gp_json_writer_err(gp_json_writer *self)
{
	return !!self->err[0];
}

/**
 * @brief Starts an json object.
 */
int gp_json_obj_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes an json object.
 */
int gp_json_obj_finish(gp_json_writer *self);

/**
 * @brief Starts an json array.
 */
int gp_json_arr_start(gp_json_writer *self, const char *id);

/**
 * @brief Finishes an json array.
 */
int gp_json_arr_finish(gp_json_writer *self);

/**
 * @brief Adds a null value.
 */
int gp_json_null_add(gp_json_writer *self, const char *id);

/**
 * @brief Adds an integer value.
 */
int gp_json_int_add(gp_json_writer *self, const char *id, long val);

/**
 * @brief Adds a bool value.
 */
int gp_json_bool_add(gp_json_writer *self, const char *id, int val);

/**
 * @brief Adds a float value.
 */
int gp_json_float_add(gp_json_writer *self, const char *id, double val);

/**
 * @brief Adds a string value.
 */
int gp_json_str_add(gp_json_writer *self, const char *id, const char *str);

/**
 * @brief Finalizes json writer.
 *
 * Finalizes the json writer, throws possible errors through the error printing
 * function.
 *
 * @self A json writer.
 * @return Overall error value.
 */
int gp_json_finish(gp_json_writer *self);

#endif /* GP_JSON_WRITER_H */
