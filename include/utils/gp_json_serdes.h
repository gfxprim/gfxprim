// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_json_serdes.h
 * @brief C structure to JSON serializer and deserializer.
 *
 * How to read a C struct from a JSON file:
 * @code
 * struct connection {
 *         char *username;
 *         char *password;
 *         char *server_addr;
 *         uint16_t port;
 * };
 *
 * static struct gp_json_obj_attr connection_attrs[] = {
 *         GP_JSON_SERDES_STR_DUP(struct connection, username, 0, 1024),
 *         GP_JSON_SERDES_STR_DUP(struct connection, password, 0, 1024),
 *         GP_JSON_SERDES_UINT16(struct connection, port, GP_JSON_SERDES_OPTIONAL, 0, UINT16_MAX),
 *         GP_JSON_SERDES_STR_DUP(struct connection, server_addr, 0, 1024, "sever address"),
 *         {}
 * };
 *
 * ...
 *          struct connnection conn = {.port = 6666};
 *
 *	    if (gp_json_load_struct(path, connection_attrs, &conn)) {
 *                  printf("Failed to load '%s'", path);
 *                  return 1;
 *	    }
 * ...
 *
 * @endcode
 */

#ifndef GP_JSON_SERDES_H
#define GP_JSON_SERDES_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include <utils/gp_json_common.h>

/** @brief Value types, the complete type is determinted by both value and size. */
enum json_serdes_type {
	GP_JSON_SERDES_STR,
	GP_JSON_SERDES_UINT,
	GP_JSON_SERDES_INT,
	GP_JSON_SERDES_FLOAT,

	/** If set parameter does not have to be present */
	GP_JSON_SERDES_OPTIONAL = 0x80,
};

#define GP_JSON_SERDES_TYPE(type) ((type) & 0x0f)

/** @brief Serializer and deserializer integer limits */
struct gp_json_int_limits {
	int64_t min;
	uint64_t max;
};

/** @brief Serializer and deserializer floating point limits */
struct gp_json_float_limits {
	float min;
	float max;
};

/**
 * @brief Describe a single structure member for serializer and deserialzer.
 *
 * Serialization and deserialization functions take an array sorted by id of
 * these structures that describe subset of a structure memebers.
 *
 * The array is usually constructed with the help of the GP_JSON_SERDES_FOO()
 * macros.
 */
typedef struct gp_json_struct {
	/** @brief JSON id */
	const char *id;
	/** @brief Offset in the C structure */
	size_t offset;

	/** @brief Variable type */
	enum json_serdes_type type;
	/** @brief Variable size, either explicit size or sizeof() */
	size_t type_size;

	/** @brief Optional value limits */
	union {
		struct gp_json_int_limits lim_int;
		struct gp_json_float_limits lim_float;
		size_t str_max_size;
	};
} gp_json_struct;

#define GP_2(P1, P2, ...) P2

/**
 * @brief Defines a string array.
 *
 * The memb has to be a fixed size char array.
 *
 * @param struct A structure type.
 * @param memb A fixed size char array member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param size The array size.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         char str_arr[128];
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_STR_CPY(struct foo, str_arr, 0, 128),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_STR_CPY(struct, memb, flags, size, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_STR | flags, \
	 .type_size = size}

/**
 * @brief Defines a string pointer.
 *
 * The memb has to be a char * pointer. The string is allocated with strdup()
 * and has to be later freed with free().
 *
 * @param struct A structure type.
 * @param memb A char * member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param max_size The maximal string size.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         char *str;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_STR_DUP(struct foo, str, 0, 128),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_STR_DUP(struct, memb, flags, max_size, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_STR | flags, \
	 .str_max_size = max_size}

/**
 * @brief Defines an integer.
 *
 * The memb has to be an int type.
 *
 * @param struct A structure type.
 * @param memb An integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass INT_MIN for full range.
 * @param max An upper limit, pass INT_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         int i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_INT(struct foo, i, 0, INT_MIN, INT_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_INT(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = sizeof(int), \
	 .lim_int = {min, max}}

/**
 * @brief Defines an unsigned integer.
 *
 * The memb has to be an unsigned int type.
 *
 * @param struct A structure type.
 * @param memb An unsigned integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass 0 for full range.
 * @param max An upper limit, pass UINT_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         unsigned int i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_UINT(struct foo, i, 0, 0, UINT_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_UINT(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = sizeof(unsigned int), \
	 .lim_int = {min, max}}

/**
 * @brief Defines a long integer.
 *
 * The memb has to be a long type.
 *
 * @param struct A structure type.
 * @param memb A long integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass LONG_MIN for full range.
 * @param max An upper limit, pass LONG_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         long l;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_LONG(struct foo, l, 0, LONG_MIN, LONG_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_LONG(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = sizeof(long), \
	 .lim_int = {min, max}}

/**
 * @brief Defines an unsigned long integer.
 *
 * The memb has to be an unsigned long type.
 *
 * @param struct A structure type.
 * @param memb An unsigned long integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass 0 for full range.
 * @param max An upper limit, pass ULONG_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         unsigned long l;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_ULONG(struct foo, l, 0, 0, ULONG_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_ULONG(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = sizeof(unsigned long), \
	 .lim_int = {min, max}}

/**
 * @brief Defines a long long integer.
 *
 * The memb has to be a long long type.
 *
 * @param struct A structure type.
 * @param memb A long long integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass LLONG_MIN for full range.
 * @param max An upper limit, pass LLONG_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         long long l;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_LONG(struct foo, l, 0, LLONG_MIN, LLONG_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_LLONG(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = sizeof(long long), \
	 .lim_int = {min, max}}

/**
 * @brief Defines an unsigned long long integer.
 *
 * The memb has to be an unsigned long long type.
 *
 * @param struct A structure type.
 * @param memb An unsigned long integer member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass 0 for full range.
 * @param max An upper limit, pass ULLONG_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         unsigned long long l;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_ULLONG(struct foo, l, 0, 0, ULLONG_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_ULLONG(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = sizeof(unsigned long long), \
	 .lim_int = {min, max}}

/**
 * @brief Defines a 8-bit integer.
 *
 * The memb has to be an int8_t type.
 *
 * @param struct A structure type.
 * @param memb An int8_t member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass INT8_MIN for full range.
 * @param max An upper limit, pass INT8_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         int8_t i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_INT8(struct foo, i, 0, INT8_MIN, INT8_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_INT8(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = 1, \
	 .lim_int = {min, max}}

/**
 * @brief Defines an unsigned 8-bit integer.
 *
 * The memb has to be an uint8_t type.
 *
 * @param struct A structure type.
 * @param memb An uint8_t member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass 0 for full range.
 * @param max An upper limit, pass UINT8_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         uint8_t i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_UINT8(struct foo, i, 0, 0, UINT8_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_UINT8(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = 1, \
	 .lim_int = {min, max}}

/**
 * @brief Defines a 16-bit integer.
 *
 * The memb has to be an int16_t type.
 *
 * @param struct A structure type.
 * @param memb An int16_t member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass INT16_MIN for full range.
 * @param max An upper limit, pass INT16_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         int16_t i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_INT16(struct foo, i, 0, INT16_MIN, INT16_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_INT16(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = 2, \
	 .lim_int = {min, max}}

/**
 * @brief Defines an unsigned 16-bit integer.
 *
 * The memb has to be an uint16_t type.
 *
 * @param struct A structure type.
 * @param memb An uint16_t member of the structure.
 * @param flags Can be GP_JSON_SERDES_OPTIONAL if value does not have to be present.
 * @param min A lower limit, pass 0 for full range.
 * @param max An upper limit, pass UINT16_MAX for full range.
 * @param ... Optional JSON id, if not set the member name is used instead.
 *
 * Example use:
 * @code
 * struct foo {
 *         ...
 *         uint16_t i;
 *         ...
 * };
 *
 * static const gp_json_struct struct_desc[] = {
 *         ...
 *         GP_JSON_SERDES_UINT16(struct foo, i, 0, 0, UINT16_MAX),
 *         ...
 * };
 * @endcode
 */
#define GP_JSON_SERDES_UINT16(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = 2, \
	 .lim_int = {min, max}}

#define GP_JSON_SERDES_INT32(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = 4, .lim_int = {min, max}}

#define GP_JSON_SERDES_UINT32(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = 4, \
	 .lim_int = {min, max}}

#define GP_JSON_SERDES_INT64(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_INT | flags, \
	 .type_size = 8, \
	 .lim_int = {min, max}}

#define GP_JSON_SERDES_UINT64(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_UINT | flags, \
	 .type_size = 8, \
	 .lim_int = {min, max}}

#define GP_JSON_SERDES_FLOAT(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_FLOAT | flags, \
	 .type_size = sizeof(float), \
	 .lim_float = {min, max}}

#define GP_JSON_SERDES_DOUBLE(struct, memb, flags, min, max, ...) \
	{.id = GP_2(dummy, ##__VA_ARGS__, #memb), \
	 .offset = offsetof(struct, memb), \
	 .type = GP_JSON_SERDES_FLOAT | flags, \
	 .type_size = sizeof(double), \
	 .lim_float = {min, max}}

/**
 * @brief Deserializes a JSON object into a C structure
 *
 * Consumes and JSON object and deserializes it into a C structure.
 *
 * This function can be mixed with the rest of the JSON parser functions.
 *
 * @param json A json reader.
 * @param val A json reader value.
 * @param desc An alphabetically sorted by id and NULL id terminatred array of
 *             structure member descriptions.
 * @param baseptr A pointer to the deserialized C structure.
 */
int gp_json_read_struct(gp_json_reader *json, gp_json_val *val,
                        const gp_json_struct *desc, void *baseptr);

/**
 * @brief Deserializes a JSON object into a C structure
 *
 * This is a simplified interface that loads just a single structure from a file.
 *
 * @param path A path to a file.
 * @param desc An alphabetically sorted by id and NULL id terminatred array of
 *       structure member descriptions.
 * @param baseptr A pointer to the deserialized C structure.
 */
int gp_json_load_struct(const char *path,
                        const gp_json_struct *desc, void *baseptr);

/**
 * @brief Serializes a C structure into a JSON object
 *
 * This function can be mixed with the rest of the JSON writer functions.
 *
 * @param json A json writer
 * @param desc An NULL id terminated array of structure member descriptions.
 * @param id An JSON id for the object, should be NULL if there is no id required in
 *        current context, e.g. inside of an JSON array.
 * @baseptr A pointer to the serialized C structure.
 */
int gp_json_write_struct(gp_json_writer *json, const gp_json_struct *desc,
                         const char *id, void *baseptr);

/**
 * @brief Serializes a C structure to JSON and saves the result into a file.
 *
 * This is a simplified interface that saves just a single structure to a file.
 *
 * @param path A path to a file.
 * @param desc An alphabetically sorted by id and NULL id terminatred array of
 *       structure member descriptions.
 * @param baseptr A pointer to the serialized C structure.
 */
int gp_json_save_struct(const char *path,
                        const gp_json_struct *desc, void *baseptr);

#endif /* GP_JSON_SERDES_H */
