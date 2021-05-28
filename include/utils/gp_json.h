// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_JSON_H
#define GP_JSON_H

#include <stdio.h>

#define GP_JSON_ERR_MAX 128
#define GP_JSON_ID_MAX 64

#define GP_JSON_RECURSION_MAX 128

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

typedef struct gp_json_buf {
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

	/** Where to print errors and warnings */
	FILE *msgf;

	char err[GP_JSON_ERR_MAX];
	char buf[];
} gp_json_buf;

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

	float val_float;

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
 * @brief Fills the buffer error.
 *
 * Once buffer error is set all parsing functions return immediatelly with type
 * set to GP_JSON_VOID.
 *
 * @buf An gp_json buffer
 * @fmt A printf like format string
 * @... A printf like parameters
 */
void gp_json_err(struct gp_json_buf *buf, const char *fmt, ...)
               __attribute__((format (printf, 2, 3)));

/*
 * @brief Prints error into a file.
 *
 * The error takes into consideration the current offset in the buffer and
 * prints a few preceding lines along with the exact position of the error.
 *
 * @f A file to print the error to.
 * @buf An gp_json buffer.
 */
void gp_json_err_print(struct gp_json_buf *buf);

/*
 * @brief Prints a warning.
 *
 * @f A file to print the error to.
 * @buf An gp_json buffer.
 */
void gp_json_warn(struct gp_json_buf *buf, const char *fmt, ...)
                  __attribute__((format (printf, 2, 3)));

/*
 * @brief Returns true if error was encountered.
 *
 * @bfu An gp_json buffer.
 * @return True if error was encountered false otherwise.
 */
static inline int gp_json_is_err(struct gp_json_buf *buf)
{
	return !!buf->err[0];
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
 * @buf An gp_json buffer.
 * @return A type of next element in the buffer.
 */
enum gp_json_type gp_json_next_type(struct gp_json_buf *buf);

/*
 * @brief Returns if first element in JSON is object or array.
 *
 * @buf An gp_json buffer.
 * @return On success returns GP_JSON_OBJ or GP_JSON_ARR. On failure GP_JSON_VOID.
 */
enum gp_json_type gp_json_start(struct gp_json_buf *buf);

/*
 * @brief Starts parsing of an JSON object.
 *
 * @buf An gp_json buffer.
 * @res An gp_json result.
 */
int gp_json_obj_first(struct gp_json_buf *buf, struct gp_json_val *res);
int gp_json_obj_next(struct gp_json_buf *buf, struct gp_json_val *res);

#define GP_JSON_OBJ_FOREACH(buf, res) \
	for (gp_json_obj_first(buf, res); gp_json_valid(res); gp_json_obj_next(buf, res))

/*
 * @brief Utility function for n*log(n) lookup in a sorted array.
 *
 * @list Analphabetically sorted array.
 * @list_len Array length.
 *
 * @return An array index or (size_t)-1 if key wasn't found.
 */
size_t gp_json_arr_lookup(const void *list, size_t memb_size, size_t list_len,
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
	return gp_json_arr_lookup(obj->attrs, sizeof(*obj->attrs), obj->attr_cnt, key);
}

#define GP_JSON_OBJ_ATTR(keyv, typev) \
	{.key = keyv, .type = typev}

/*
 * @brief Object parsing functions with with attribute list.
 *
 * These functions allows you to efficiently filter a set of keys
 * for a given object passed in gp_json_obj.
 *
 * @buf An gp_json buffer.
 * @res An gp_json result.
 * @obj An gp_json_obj object description.
 * @f A file to print warnings to.
 */
int gp_json_obj_first_filter(struct gp_json_buf *buf, struct gp_json_val *res,
                             const struct gp_json_obj *obj, const struct gp_json_obj *ign);
int gp_json_obj_next_filter(struct gp_json_buf *buf, struct gp_json_val *res,
                            const struct gp_json_obj *obj, const struct gp_json_obj *ign);

#define GP_JSON_OBJ_FILTER(buf, res, obj, ign) \
	for (gp_json_obj_first_filter(buf, res, obj, ign); gp_json_valid(res); gp_json_obj_next_filter(buf, res, obj, ign))

/*
 * @brief Skips parsing of an JSON object.
 *
 * @buf An gp_json buffer.
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_obj_skip(struct gp_json_buf *buf);

int gp_json_arr_first(struct gp_json_buf *buf, struct gp_json_val *res);
int gp_json_arr_next(struct gp_json_buf *buf, struct gp_json_val *res);

#define GP_JSON_ARR_FOREACH(buf, res) \
	for (gp_json_arr_first(buf, res); gp_json_valid(res); gp_json_arr_next(buf, res))

/*
 * @brief Skips parsing of an JSON array.
 *
 * @buf An gp_json buffer.
 * @return Zero on success, non-zero otherwise.
 */
int gp_json_arr_skip(struct gp_json_buf *buf);

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
 * @buf An gp_json buffer.
 * @return A state that points to a start of the last object or array.
 */
static inline struct gp_json_state gp_json_state_start(struct gp_json_buf *buf)
{
	struct gp_json_state ret = {
		.off = buf->sub_off,
		.depth = buf->depth,
	};

	return ret;
}

/*
 * @brief Returns the parser to a saved state.
 *
 * This function could be used for the parser to return to the start of
 * object or array saved by t the gp_json_state_get() function.
 *
 * @buf An gp_json buffer.
 * @state An parser state as returned by the gp_json_state_get().
 */
static inline void gp_json_state_load(struct gp_json_buf *buf, struct gp_json_state state)
{
	buf->off = state.off;
	buf->sub_off = state.off;
	buf->depth = state.depth;
}

/*
 * @brief Loads a file into an gp_json buffer.
 *
 * @path A path to a file.
 * @return An gp_json buffer or NULL in a case of a failure.
 */
struct gp_json_buf *gp_json_load(const char *path);

/*
 * @brief Frees an gp_json buffer.
 *
 * @buf An gp_json buffer allcated by gp_json_load() function.
 */
void gp_json_free(struct gp_json_buf *buf);

/*
 * @brief Returns non-zero if whole buffer has been consumed.
 *
 * @buf And gp_json buffer.
 */
static inline int gp_json_empty(struct gp_json_buf *buf)
{
	return buf->off >= buf->len;
}

#endif /* GP_JSON_H */
