// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_data_storage.h
 * @brief Typed recursive storage for key value pairs.
 */

#ifndef LOADERS_DATA_STORAGE_H
#define LOADERS_DATA_STORAGE_H

/**
 * @brief Data type.
 */
enum gp_data_type {
	/** @brief A long integer. */
	GP_DATA_INT,
	/** @brief A string. */
	GP_DATA_STRING,
	/** @brief A double. */
	GP_DATA_DOUBLE,
	/** @brief A rational number. */
	GP_DATA_RATIONAL,
	/** @brief A dictionary aka hash. */
	GP_DATA_DICT,
};

/**
 * @brief Rational number.
 */
struct gp_data_rational {
	/** @brief Numerator. */
	long num;
	/** @brief Denominator. */
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

/**
 * @brief A data node.
 */
typedef struct gp_data_node {
	/** @brief A data type. */
	enum gp_data_type type;
	/** @brief A data value. */
	union gp_data_value value;
	/** @brief A data id. */
	const char *id;
	/** @brief Next node in the current dict. */
	struct gp_data_node *next;
} gp_data_node;

/**
 * @brief Creates an empty data storage
 *
 * @return A newly created data storage or NULL in a case of a failure.
 */
gp_storage *gp_storage_create(void);

/**
 * @brief Destroys a data storage and frees all its data.
 *
 * @param self A data storage to be freed.
 */
void gp_storage_destroy(gp_storage *self);

/**
 * @brief Returns storage root node.
 *
 * The root node in data storage is always a dictionary.
 *
 * @param self A data storage.
 *
 * @return Data storage root.
 */
gp_data_node *gp_storage_root(gp_storage *self);

/**
 * @brief Returns first node in a dict node list.
 *
 * The rest of the nodes are linked via the gp_data_node::next pointer.
 *
 * @param node A dict type node.
 * @return A first node in the dict node, may be NULL if empty.
 */
gp_data_node *gp_data_dict_first(gp_data_node *node);

/**
 * @brief Clears all data in storage.
 *
 * After this call the data storage is empty as it was just created with
 * gp_storage_create().
 *
 * @param self A data storage.
 */
void gp_storage_clear(gp_storage *self);

/**
 * @brief Prints a data node into a stdout.
 *
 * @param node A data node.
 */
void gp_data_print(const gp_data_node *node);

/**
 * @brief Prints a content of a storage into the stdout.
 *
 * @param self A data storage.
 */
static inline void gp_storage_print(gp_storage *self)
{
	gp_data_print(gp_storage_root(self));
}

/**
 * @brief Returns a data type name.
 *
 * @param type A data type.
 *
 * @return Returns a data type name.
 */
const char *gp_data_type_name(enum gp_data_type type);

/*
 * Returns subnode of a given id (or NULL) starting at node.
 *
 * If node is NULL storage root is used.
 */
gp_data_node *gp_storage_get(gp_storage *self,
		             gp_data_node *node, const char *id);

/**
 * @brief Returns data node by a path in the data storage.
 *
 * Example path: "/Exif/Orientation"
 *
 * The path works like filesystem path. The equivalent to working
 * directory is the node pointer.
 *
 * @param self A storage for the lookup.
 * @param node A node in the storage to start the lookup at, if NULL root node
 *             is used.
 * @param path A path to lookup, e.g. "/Exif/Orientation".
 *
 * @return A node we were looking for or NULL if not found.
 */
gp_data_node *gp_storage_get_by_path(gp_storage *self, gp_data_node *node,
                                     const char *path);

/**
 * @brief Adds data into a dict.
 *
 * The data holds information to be used for the node addition and the data are
 * copied into a newly created node.
 *
 * The call may if the node to add the data into has wrong type i.e. is not a
 * GP_DATA_DICT, if the key supplied in the data::id already exists in the
 * node, or if allocation has failed.
 *
 * @param self A storage to add the data into.
 * @param node A node to add the data into, if NULL root is used.
 * @param data A data to be added.
 * @return Newly created node or NULL in case of failure.
 */
gp_data_node *gp_storage_add(gp_storage *self,
		             gp_data_node *node, gp_data_node *data);

/**
 * @brief Adds a long integer to a storage.
 *
 * Calls gp_storage_add() internally.
 *
 * @param self A storage to add the data to.
 * @param node A dict node to add the data to, if NULL data are added to root
 *             node.
 * @param id A name for the newly added node.
 * @param i A value for the newly added node.
 *
 * @return A pointer to a newly added node or NULL in a case of a failure.
 */
gp_data_node *gp_storage_add_int(gp_storage *self, gp_data_node *node,
                                 const char *id, long i);

/**
 * @brief Adds a string to a storage.
 *
 * Calls gp_storage_add() internally.
 *
 * @param self A storage to add the data to.
 * @param node A dict node to add the data to, if NULL data are added to root
 *             node.
 * @param id A name for the newly added node.
 * @param str A value for the newly added node.
 *
 * @return A pointer to a newly added node or NULL in a case of a failure.
 */
gp_data_node *gp_storage_add_string(gp_storage *self, gp_data_node *node,
                                    const char *id, const char *str);

/**
 * @brief Adds a double floating point to a storage.
 *
 * Calls gp_storage_add() internally.
 *
 * @param self A storage to add the data to.
 * @param node A dict node to add the data to, if NULL data are added to root
 *             node.
 * @param id A name for the newly added node.
 * @param d A value for the newly added node.
 *
 * @return A pointer to a newly added node or NULL in a case of a failure.
 */
gp_data_node *gp_storage_add_double(gp_storage *self, gp_data_node *node,
                                    const char *id, double d);

/**
 * @brief Adds a rational number to a storage.
 *
 * Calls gp_storage_add() internally.
 *
 * @param self A storage to add the data to.
 * @param node A dict node to add the data to, if NULL data are added to root
 *             node.
 * @param id A name for the newly added node.
 * @param num A numerator value for the newly added node.
 * @param den A denominator value for the newly added node.
 *
 * @return A pointer to a newly added node or NULL in a case of a failure.
 */
gp_data_node *gp_storage_add_rational(gp_storage *self, gp_data_node *node,
                                      const char *id, long num, long den);

/**
 * @brief Adds a dictionary to a storage.
 *
 * Calls gp_storage_add() internally.
 *
 * @param self A storage to add the data to.
 * @param node A dict node to add the data to, if NULL data are added to root
 *             node.
 * @param id A name for the newly added node.
 *
 * @return A pointer to a newly added node or NULL in a case of a failure.
 */
gp_data_node *gp_storage_add_dict(gp_storage *self, gp_data_node *node,
                                  const char *id);

#endif /* LOADERS_GP_DATA_STORAGE_H */
