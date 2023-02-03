// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_JSON_READER_H
#define GP_JSON_READER_H

#include <stdio.h>
#include <utils/gp_json_common.h>

#define GP_JSON_READER_INIT(buf, buf_len) { \
	.max_depth = GP_JSON_RECURSION_MAX, \
	.err_print = GP_JSON_ERR_PRINT, \
	.err_print_priv = GP_JSON_ERR_PRINT_PRIV, \
	.json = buf, \
	.len = buf_len, \
}

struct gp_json_reader {
	/** Pointer to a null terminated JSON string */
	const char *json;
	/** A length of the JSON string */
	size_t len;
	/** A current offset into the JSON string */
	size_t off;
	/** An offset to the start of the last array or object */
	size_t sub_off;
	/** Recursion depth increased when array/object is entered decreased on leave */
	unsigned int depth;
	/** Maximal recursion depth */
	unsigned int max_depth;

	/** Handler to print errors and warnings */
	void (*err_print)(void *err_print_priv, const char *line);
	void *err_print_priv;

	char err[GP_JSON_ERR_MAX];
	char buf[];
};

typedef struct gp_json_val {
	enum gp_json_type type;

	/** An user supplied buffer and size to store a string values to. */
	char *buf;
	size_t buf_size;

	/** An index to attribute list, set by the foo_obj_filter() functions */
	size_t idx;

	/** An union to store the parsed value into. */
	union {
		int val_bool;
		long val_int;
		const char *val_str;
	};

	double val_float;

	/** An ID for object values */
	char id[GP_JSON_ID_MAX];
} gp_json_val;

/*
 * @brief Returns type name.
 *
 * @type A json type.
 * @return A type name.
 */
const char *gp_json_type_name(enum gp_json_type type);

/*
 * @brief Fills the reader error.
 *
 * Once buffer error is set all parsing functions return immediatelly with type
 * set to GP_JSON_VOID.
 *
 * @self An gp_json_reader
 * @fmt A printf like format string
 * @... A printf like parameters
 */
void gp_json_err(gp_json_reader *self, const char *fmt, ...)
               __attribute__((format (printf, 2, 3)));

/*
 * @brief Prints error stored in the buffer.
 *
 * The error takes into consideration the current offset in the buffer and
 * prints a few preceding lines along with the exact position of the error.
 *
 * The error is passed to the err_print() handler.
 *
 * @self A gp_json_reader
 */
void gp_json_err_print(gp_json_reader *self);

/*
 * @brief Prints a warning.
 *
 * Uses the print handler in the buffer to print a warning along with a few
 * lines of context from the JSON at the current position.
 *
 * @self A gp_json_reader
 * @fmt A printf-like error string.
 * @... A printf-like parameters.
 */
void gp_json_warn(gp_json_reader *self, const char *fmt, ...)
                  __attribute__((format (printf, 2, 3)));

/*
 * @brief Returns true if error was encountered.
 *
 * @self A gp_json_reader
 * @return True if error was encountered false otherwise.
 */
static inline int gp_json_reader_err(gp_json_reader *self)
{
	return !!self->err[0];
}

/*
 * @brief Checks is result has valid type.
 *
 * @res An gp_json value.
 * @return Zero if result is not valid, non-zero otherwise.
 */
static inline int gp_json_valid(struct gp_json_val *res)
{
	return !!res->type;
}

/*
 * @brief Returns the type of next element in buffer.
 *
 * @self A gp_json_reader
 * @return A type of next element in the buffer.
 */
enum gp_json_type gp_json_next_type(gp_json_reader *self);

/*
 * @brief Returns if first element in JSON is object or array.
 *
 * @self A gp_json_reader
 * @return On success returns GP_JSON_OBJ or GP_JSON_ARR. On failure GP_JSON_VOID.
 */
enum gp_json_type gp_json_start(gp_json_reader *self);

/*
 * @brief Starts parsing of an JSON object.
 *
 * @self A gp_json_reader
 * @res An gp_json result.
 */
int gp_json_obj_first(gp_json_reader *self, struct gp_json_val *res);
int gp_json_obj_next(gp_json_reader *self, struct gp_json_val *res);

#define GP_JSON_OBJ_FOREACH(buf, res) \
	for (gp_json_obj_first(buf, res); gp_json_valid(res); gp_json_obj_next(buf, res))

/*
 * @brief Utility function for log(n) lookup in a sorted array.
 *
 * @list Analphabetically sorted array.
 * @list_len Array length.
 *
 * @return An array index or (size_t)-1 if key wasn't found.
 */
size_t gp_json_lookup(const void *arr, size_t memb_size, size_t list_len,
                      const char *key);

/** An object attribute description i.e. key and type */
typedef struct gp_json_obj_attr {
	const char *key;
	enum gp_json_type type;
} gp_json_obj_attr;

/** An object description */
typedef struct gp_json_obj {
	/** Attributes we are looking for for these the val->idx is set */
	const gp_json_obj_attr *attrs;
	size_t attr_cnt;
} gp_json_obj;

static inline size_t gp_json_obj_lookup(const gp_json_obj *obj, const char *key)
{
	return gp_json_lookup(obj->attrs, sizeof(*obj->attrs), obj->attr_cnt, key);
}

#define GP_JSON_OBJ_ATTR(keyv, typev) \
	{.key = keyv, .type = typev}

/*
 * @brief Object parsing functions with with attribute list.
 *
 * These functions allows you to efficiently filter a set of keys
 * for a given object passed in gp_json_obj.
 *
 * @self A gp_json_reader
 * @res An gp_json result.
 * @obj An gp_json_obj object description.
 * @f A file to print warnings to.
 */
int gp_json_obj_first_filter(gp_json_reader *self, struct gp_json_val *res,
                             const struct gp_json_obj *obj, const struct gp_json_obj *ign);
int gp_json_obj_next_filter(gp_json_reader *self, struct gp_json_val *res,
                            const struct gp_json_obj *obj, const struct gp_json_obj *ign);

#define GP_JSON_OBJ_FILTER(buf, res, obj, ign) \
	for (gp_json_obj_first_filter(buf, res, obj, ign); gp_json_valid(res); gp_json_obj_next_filter(buf, res, obj, ign))

/*
 * @brief Skips parsing of an JSON object.
 *
 * @self A gp_json_reader
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_skip(gp_json_reader *self);

int gp_json_arr_first(gp_json_reader *self, struct gp_json_val *res);
int gp_json_arr_next(gp_json_reader *self, struct gp_json_val *res);

#define GP_JSON_ARR_FOREACH(buf, res) \
	for (gp_json_arr_first(buf, res); gp_json_valid(res); gp_json_arr_next(buf, res))

/*
 * @brief Skips parsing of an JSON array.
 *
 * @self A gp_json_reader
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_skip(gp_json_reader *self);

typedef struct gp_json_state {
	size_t off;
	unsigned int depth;
} gp_json_state;

/*
 * @brief Returns a parser state at the start of current object/array.
 *
 * This function could be used for the parser to return to the start of the
 * currently parsed object or array.
 *
 * @self A gp_json_reader
 * @return A state that points to a start of the last object or array.
 */
static inline struct gp_json_state gp_json_state_start(gp_json_reader *self)
{
	struct gp_json_state ret = {
		.off = self->sub_off,
		.depth = self->depth,
	};

	return ret;
}

/*
 * @brief Returns the parser to a saved state.
 *
 * This function could be used for the parser to return to the start of
 * object or array saved by t the gp_json_state_get() function.
 *
 * @self A gp_json_reader
 * @state An parser state as returned by the gp_json_state_get().
 */
static inline void gp_json_state_load(gp_json_reader *self, struct gp_json_state state)
{
	self->off = state.off;
	self->sub_off = state.off;
	self->depth = state.depth;
}

/**
 * @brief Resets the parser to a start.
 *
 * @self A gp_json_reader
 */
static inline void gp_json_reset(gp_json_reader *self)
{
	self->off = 0;
	self->sub_off = 0;
	self->depth = 0;
	self->err[0] = 0;
}

/*
 * @brief Loads a file into an gp_json buffer.
 *
 * @path A path to a file.
 * @return A gp_json_reader or NULL in a case of a failure.
 */
gp_json_reader *gp_json_reader_load(const char *path);

/*
 * @brief Frees an gp_json buffer.
 *
 * @self A gp_json_reader
 * @buf A gp_json_reader allcated by gp_json_load() function.
 */
void gp_json_reader_free(gp_json_reader *self);

/**
 * @brief Prints errors and warnings at the end of parsing.
 *
 * Checks if self->err is set and prints the error with gp_json_reader_err()
 *
 * Checks if there is any text left after the parser has finished and prints
 * a warning if so.
 *
 * @self A gp_json_reader
 */
void gp_json_reader_finish(gp_json_reader *self);

/*
 * @brief Returns non-zero if whole buffer has been consumed.
 *
 * @self A gp_json_reader
 * @buf And gp_json buffer.
 */
static inline int gp_json_empty(gp_json_reader *self)
{
	return self->off >= self->len;
}

#endif /* GP_JSON_H */
