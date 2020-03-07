// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Data storage is an abstraction for recusive, typed, name -> value pairs.

   It's designed to be common layer for metadata such as exif, etc.

 */

#ifndef LOADERS_DATA_STORAGE_H
#define LOADERS_DATA_STORAGE_H

enum gp_data_type {
	GP_DATA_INT,
	GP_DATA_STRING,
	GP_DATA_DOUBLE,
	GP_DATA_RATIONAL,
	GP_DATA_DICT,
};

struct gp_data_rational {
	long num;
	long den;
};

typedef struct gp_storage gp_storage;
typedef struct gp_data_dict gp_data_dict;

union gp_data_value {
	long i;
	double d;
	const char *str;
	struct gp_data_rational rat;
	gp_data_dict *dict;
};

typedef struct gp_data_node {
	enum gp_data_type type;
	union gp_data_value value;
	const char *id;
	struct gp_data_node *next;
} gp_data_node;

/*
 * Creates a data storage
 */
gp_storage *gp_storage_create(void);

/*
 * Destroys a data storage and all its data
 */
void gp_storage_destroy(gp_storage *self);

/*
 * Returns storage root node.
 */
gp_data_node *gp_storage_root(gp_storage *self);

/*
 * Returns first node in a dict node list.
 */
gp_data_node *gp_data_dict_first(gp_data_node *node);

/*
 * Clears all data in storage.
 */
void gp_storage_clear(gp_storage *self);

/*
 * Prints a data
 */
void gp_data_print(const gp_data_node *node);

static inline void gp_storage_print(gp_storage *self)
{
	gp_data_print(gp_storage_root(self));
}

const char *gp_data_type_name(enum gp_data_type type);

/*
 * Returns subnode of a given id (or NULL) starting at node.
 *
 * If node is NULL storage root is used.
 */
gp_data_node *gp_storage_get(gp_storage *self,
		             gp_data_node *node, const char *id);

/*
 * Returns data node by a path in the data storage.
 *
 * Example path: "/Exif/Orientation"
 *
 * The path works like filesystem path. The equivalent to working
 * directory is the node pointer.
 */
gp_data_node *gp_storage_get_by_path(gp_storage *self, gp_data_node *node,
                                     const char *path);

/*
 * Adds data into a dict.
 *
 * If node is NULL, data storage root is used.
 *
 * The data holds information to be used for the node addition.
 *
 * Returns newly created node or NULL in case of failure.
 */
gp_data_node *gp_storage_add(gp_storage *self,
		             gp_data_node *node, gp_data_node *data);

/*
 * Shortcuts for adding a int/string/double/rational
 *
 * If dict is NULL data is added to the root.
 */
gp_data_node *gp_storage_add_int(gp_storage *self, gp_data_node *node,
                                 const char *id, long i);

gp_data_node *gp_storage_add_string(gp_storage *self, gp_data_node *node,
                                    const char *id, const char *str);

gp_data_node *gp_storage_add_double(gp_storage *self, gp_data_node *node,
                                    const char *id, double d);

gp_data_node *gp_storage_add_rational(gp_storage *self, gp_data_node *node,
                                      const char *id, long num, long den);

gp_data_node *gp_storage_add_dict(gp_storage *self, gp_data_node *node,
                                  const char *id);

#endif /* LOADERS_GP_DATA_STORAGE_H */
