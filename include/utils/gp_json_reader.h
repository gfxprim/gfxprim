// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_json_reader.h
 * @brief A recursive descend JSON parser.
 *
 * All the function that parse JSON return zero on success and non-zero on a
 * failure. Once an error has happened all subsequent attempts to parse more
 * return with non-zero exit status immediatelly. This is designed so that we
 * can parse several values without checking each return value and only check
 * if error has happened at the end of the sequence.
 */

#ifndef GP_JSON_READER_H
#define GP_JSON_READER_H

#include <stdio.h>
#include <core/gp_compiler.h>
#include <utils/gp_json_common.h>

/**
 * @brief A gp_json_reader initializer with default values.
 *
 * @param buf A pointer to a buffer with JSON data.
 * @param buf_len A JSON data buffer lenght.
 *
 * @return A gp_json_reader initialized with default values.
 */
#define GP_JSON_READER_INIT(buf, buf_len) { \
	.max_depth = GP_JSON_RECURSION_MAX, \
	.err_print = GP_JSON_ERR_PRINT, \
	.err_print_priv = GP_JSON_ERR_PRINT_PRIV, \
	.json = buf, \
	.len = buf_len, \
}

/**
 * @brief A JSON parser internal state.
 */
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

/**
 * @brief A parsed JSON key value pair.
 */
struct gp_json_val {
	/**
	 * @brief A value type.
	 *
	 * GP_JSON_VALUE_VOID means that no value was parsed.
	 */
	enum gp_json_type type;

	/** An user supplied buffer and size to store a string values to. */
	char *buf;
	size_t buf_size;

	/**
	 * An index to attribute list.
	 *
	 * This is set by the gp_json_obj_first_filter() and
         * gp_json_obj_next_filter() functions.
	 */
	size_t idx;

	/** An union to store the parsed value into. */
	union {
		/** @brief A boolean value. */
		int val_bool;
		/** @brief An integer value. */
		long long val_int;
		/** @brief A string value. */
		const char *val_str;
	};

	/**
	 * @brief A floating point value.
	 *
	 * Since integer values are subset of floating point values val_float
	 * is always set when val_int was set.
	 */
	double val_float;

	/** @brief An ID for object values */
	char id[GP_JSON_ID_MAX];

	char buf__[];
};

/**
 * @brief Allocates a JSON value.
 *
 * @param buf_size A maximal buffer size for a string value, pass 0 for default.
 * @return A newly allocated JSON value.
 */
gp_json_val *gp_json_val_alloc(size_t buf_size);

/**
 * @brief Frees a JSON value.
 *
 * @param self A JSON value previously allocated by gp_json_val_alloc().
 */
void gp_json_val_free(gp_json_val *self);

/**
 * @brief Checks is result has valid type.
 *
 * @param res A gp_json value.
 * @return Zero if result is not valid, non-zero otherwise.
 */
static inline int gp_json_val_valid(struct gp_json_val *res)
{
	return !!res->type;
}

/**
 * @brief Fills the reader error.
 *
 * Once buffer error is set all parsing functions return immediatelly with type
 * set to GP_JSON_VOID.
 *
 * @param self A gp_json_reader
 * @param fmt A printf like format string
 * @param ... A printf like parameters
 */
void gp_json_err(gp_json_reader *self, const char *fmt, ...)
               GP_FMT_PRINTF(2, 3);

/**
 * @brief Prints error stored in the buffer.
 *
 * The error takes into consideration the current offset in the buffer and
 * prints a few preceding lines along with the exact position of the error.
 *
 * The error is passed to the err_print() handler.
 *
 * @param self A gp_json_reader
 */
void gp_json_err_print(gp_json_reader *self);

/**
 * @brief Prints a warning.
 *
 * Uses the print handler in the buffer to print a warning along with a few
 * lines of context from the JSON at the current position.
 *
 * @param self A gp_json_reader
 * @param fmt A printf-like error string.
 * @param ... A printf-like parameters.
 */
void gp_json_warn(gp_json_reader *self, const char *fmt, ...)
                  GP_FMT_PRINTF(2, 3);

/**
 * @brief Returns true if error was encountered.
 *
 * @param self A gp_json_reader
 * @return True if error was encountered false otherwise.
 */
static inline int gp_json_reader_err(gp_json_reader *self)
{
	return !!self->err[0];
}

/**
 * @brief Returns the type of next element in buffer.
 *
 * @param self A gp_json_reader
 * @return A type of next element in the buffer.
 */
enum gp_json_type gp_json_next_type(gp_json_reader *self);

/**
 * @brief Returns if first element in JSON is object or array.
 *
 * @param self A gp_json_reader
 * @return On success returns GP_JSON_OBJ or GP_JSON_ARR. On failure GP_JSON_VOID.
 */
enum gp_json_type gp_json_reader_start(gp_json_reader *self);

/**
 * @brief Starts parsing of a JSON object.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_first(gp_json_reader *self, struct gp_json_val *res);

/**
 * @brief Parses next value from a JSON object.
 *
 * If the res->type is GP_JSON_OBJ or GP_JSON_ARR it has to be parsed or
 * skipped before next call to this function.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_next(gp_json_reader *self, struct gp_json_val *res);

/**
 * @brief A loop over a JSON object.
 *
 * @code
 * GP_JSON_OBJ_FOREACH(reader, val) {
 *      printf("Got value id '%s' type '%s'", val->id, gp_json_type_name(val->type));
 *      ...
 * }
 * @endcode
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 */
#define GP_JSON_OBJ_FOREACH(self, res) \
	for (gp_json_obj_first(self, res); gp_json_val_valid(res); gp_json_obj_next(self, res))

/**
 * @brief Utility function for log(n) lookup in a sorted array.
 *
 * @param list Analphabetically sorted array.
 * @param list_len Array length.
 *
 * @return An array index or (size_t)-1 if key wasn't found.
 */
size_t gp_json_lookup(const void *arr, size_t memb_size, size_t list_len,
                      const char *key);

/** @brief A JSON object attribute description i.e. key and type */
typedef struct gp_json_obj_attr {
	/** @brief A JSON object key name. */
	const char *key;
	/**
	 * @brief A JSON object value type.
	 *
	 * Note that because integer numbers are subset of floating point
	 * numbers if requested type was GP_JSON_FLOAT it will match if parsed
	 * type was GP_JSON_INT and the val_float will be set in addition to
	 * val_int.
	 */
	enum gp_json_type type;
} gp_json_obj_attr;

/** @brief A JSON object description */
typedef struct gp_json_obj {
	/**
	 * @brief A list of object attributes.
	 *
	 * Attributes we are looking for, the parser sets the val->idx for these.
	 */
	const gp_json_obj_attr *attrs;
	/** @brief A size of attrs array. */
	size_t attr_cnt;
} gp_json_obj;

static inline size_t gp_json_obj_lookup(const gp_json_obj *obj, const char *key)
{
	return gp_json_lookup(obj->attrs, sizeof(*obj->attrs), obj->attr_cnt, key);
}

/** @brief A gp_json_obj_attr initializer. */
#define GP_JSON_OBJ_ATTR(keyv, typev) \
	{.key = keyv, .type = typev}

/** @brief gp_json_obj_attr intializer with an array index. */
#define GP_JSON_OBJ_ATTR_IDX(key_idx, keyv, typev) \
        [key_idx] = {.key = keyv, .type = typev}

/**
 * @brief Starts parsing of a JSON object with attribute lists.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 * @param obj A gp_json_obj object description.
 * @param ign A list of keys to ignore.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_first_filter(gp_json_reader *self, struct gp_json_val *res,
                             const struct gp_json_obj *obj, const struct gp_json_obj *ign);

/**
 * @brief Parses next value from a JSON object with attribute lists.
 *
 * If the res->type is GP_JSON_OBJ or GP_JSON_ARR it has to be parsed or skipped
 * before next call to this function.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 * @param obj A gp_json_obj object description.
 * @param ign A list of keys to ignore.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_next_filter(gp_json_reader *self, struct gp_json_val *res,
                            const struct gp_json_obj *obj, const struct gp_json_obj *ign);

/**
 * @brief A loop over a JSON object with a pre-defined list of expected attributes.
 *
 * @code
 * static struct gp_json_obj_attr attrs[] = {
 *	GP_JSON_OBJ_ATTR("bool", GP_JSON_BOOL),
 *	GP_JSON_OBJ_ATTR("number", GP_JSON_INT),
 * };
 *
 * static struct gp_json_obj obj = {
 *	attrs = filter_attrs,
 *	.attr_cnt = GP_JSON_ARRAY_SIZE(filter_attrs)
 * };
 *
 * GP_JSON_OBJ_FOREACH_FILTER(reader, val, &obj, NULL) {
 *	printf("Got value id '%s' type '%s'",
 *	       attrs[val->idx].id, gp_json_type_name(val->type));
 *      ...
 * }
 * @endcode
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the next parsed value to.
 * @param obj A gp_json_obj with a description of attributes to parse.
 * @param ign A gp_json_obj with a description of attributes to ignore.
 */
#define GP_JSON_OBJ_FOREACH_FILTER(self, res, obj, ign) \
	for (gp_json_obj_first_filter(self, res, obj, ign); \
	     gp_json_val_valid(res); \
	     gp_json_obj_next_filter(self, res, obj, ign))

/**
 * @brief Skips parsing of a JSON object.
 *
 * @param self A gp_json_reader.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_skip(gp_json_reader *self);

/**
 * @brief Starts parsing of a JSON array.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_first(gp_json_reader *self, struct gp_json_val *res);

/**
 * @brief Parses next value from a JSON array.
 *
 * If the res->type is GP_JSON_OBJ or GP_JSON_ARR it has to be parsed or
 * skipped before next call to this function.
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the parsed value to.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_next(gp_json_reader *self, struct gp_json_val *res);

/**
 * @brief A loop over a JSON array.
 *
 * @code
 * GP_JSON_ARR_FOREACH(reader, val) {
 *      printf("Got value type '%s'", gp_json_type_name(val->type));
 *      ...
 * }
 * @endcode
 *
 * @param self A gp_json_reader.
 * @param res A gp_json_val to store the next parsed value to.
 */
#define GP_JSON_ARR_FOREACH(self, res) \
	for (gp_json_arr_first(self, res); gp_json_val_valid(res); gp_json_arr_next(self, res))

/**
 * @brief Skips parsing of a JSON array.
 *
 * @param self A gp_json_reader.
 *
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_skip(gp_json_reader *self);

/**
 * @brief A JSON parser state.
 */
typedef struct gp_json_reader_state {
	size_t off;
	unsigned int depth;
} gp_json_reader_state;

/**
 * @brief Returns a parser state at the start of current object/array.
 *
 * This function could be used for the parser to return to the start of the
 * currently parsed object or array.
 *
 * @param self A gp_json_reader
 * @return A state that points to a start of the last object or array.
 */
static inline gp_json_reader_state gp_json_reader_state_save(gp_json_reader *self)
{
	struct gp_json_reader_state ret = {
		.off = self->sub_off,
		.depth = self->depth,
	};

	return ret;
}

/**
 * @brief Returns the parser to a saved state.
 *
 * This function could be used for the parser to return to the start of
 * object or array saved by t the gp_json_reader_state_get() function.
 *
 * @param self A gp_json_reader
 * @param state An parser state as returned by the gp_json_reader_state_get().
 */
static inline void gp_json_reader_state_load(gp_json_reader *self, gp_json_reader_state state)
{
	self->off = state.off;
	self->sub_off = state.off;
	self->depth = state.depth;
}

/**
 * @brief Resets the parser to a start.
 *
 * @param self A gp_json_reader
 */
static inline void gp_json_reader_reset(gp_json_reader *self)
{
	self->off = 0;
	self->sub_off = 0;
	self->depth = 0;
	self->err[0] = 0;
}

/**
 * @brief Loads a file into an gp_json_reader buffer.
 *
 * The reader has to be later freed by gp_json_reader_free().
 *
 * @param path A path to a file.
 * @return A gp_json_reader or NULL in a case of a failure.
 */
gp_json_reader *gp_json_reader_load(const char *path);

/**
 * @brief Frees an gp_json_reader buffer.
 *
 * @param self A gp_json_reader allocated by gp_json_reader_load() function.
 */
void gp_json_reader_free(gp_json_reader *self);

/**
 * @brief Prints errors and warnings at the end of parsing.
 *
 * Checks if self->err is set and prints the error with gp_json_reader_err()
 *
 * Checks if there is any text left after the parser has finished with
 * gp_json_reader_consumed() and prints a warning if there were any
 * non-whitespace characters left.
 *
 * @param self A gp_json_reader
 */
void gp_json_reader_finish(gp_json_reader *self);

/**
 * @brief Returns non-zero if whole buffer has been consumed.
 *
 * @param self A gp_json_reader.
 * @return Non-zero if whole buffer was consumed.
 */
static inline int gp_json_reader_consumed(gp_json_reader *self)
{
	return self->off >= self->len;
}

#endif /* GP_JSON_H */
