/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

   Data storage is an abstraction for recusive, typed, name -> value pairs.

   It's designed to be common layer for metadata such as exif, etc.

 */

#ifndef LOADERS_DATA_STORAGE_H
#define LOADERS_DATA_STORAGE_H

enum GP_DataType {
	GP_DATA_INT,
	GP_DATA_STRING,
	GP_DATA_DOUBLE,
	GP_DATA_RATIONAL,
	GP_DATA_DICT,
};

struct GP_DataRational {
	long num;
	long den;
};

typedef struct GP_DataStorage GP_DataStorage;
typedef struct GP_DataDict GP_DataDict;

union GP_DataValue {
	long i;
	double d;
	const char *str;
	struct GP_DataRational rat;
	GP_DataDict *dict;
};

typedef struct GP_DataNode {
	enum GP_DataType type;
	union GP_DataValue value;
	const char *id;
	struct GP_DataNode *next;
} GP_DataNode;

/*
 * Creates a data storage
 */
GP_DataStorage *GP_DataStorageCreate(void);

/*
 * Destroys a data storage and all its data
 */
void GP_DataStorageDestroy(GP_DataStorage *self);

/*
 * Returns storage root node.
 */
GP_DataNode *GP_DataStorageRoot(GP_DataStorage *self);

/*
 * Returns first node in a dict node list.
 */
GP_DataNode *GP_DataDictFirst(GP_DataNode *node);

/*
 * Clears all data in storage.
 */
void GP_DataStorageClear(GP_DataStorage *self);

/*
 * Prints a data
 */
void GP_DataPrint(const GP_DataNode *node);

static inline void GP_DataStoragePrint(GP_DataStorage *self)
{
	GP_DataPrint(GP_DataStorageRoot(self));
}

const char *GP_DataTypeName(enum GP_DataType type);

/*
 * Returns subnode of a given id (or NULL) starting at node.
 *
 * If node is NULL storage root is used.
 */
GP_DataNode *GP_DataStorageGet(GP_DataStorage *self,
		               GP_DataNode *node, const char *id);

/*
 * Returns data node by a path in the data storage.
 *
 * Example path: "/Exif/Orientation"
 *
 * The path works like filesystem path. The equivalent to working
 * directory is the node pointer.
 */
GP_DataNode *GP_DataStorageGetByPath(GP_DataStorage *self, GP_DataNode *node,
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
GP_DataNode *GP_DataStorageAdd(GP_DataStorage *self,
		               GP_DataNode *node, GP_DataNode *data);

/*
 * Shortcuts for adding a int/string/double/rational
 *
 * If dict is NULL data is added to the root.
 */
GP_DataNode *GP_DataStorageAddInt(GP_DataStorage *self, GP_DataNode *node,
                                  const char *id, long i);

GP_DataNode *GP_DataStorageAddString(GP_DataStorage *self, GP_DataNode *node,
                                     const char *id, const char *str);

GP_DataNode *GP_DataStorageAddDouble(GP_DataStorage *self, GP_DataNode *node,
                                     const char *id, double d);

GP_DataNode *GP_DataStorageAddRational(GP_DataStorage *self, GP_DataNode *node,
                                       const char *id, long num, long den);

GP_DataNode *GP_DataStorageAddDict(GP_DataStorage *self, GP_DataNode *node,
                                   const char *id);

#endif /* LOADERS_GP_DATA_STORAGE_H */
