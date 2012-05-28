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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef LOADERS_METADATA_H
#define LOADERS_METADATA_H

#define GP_META_RECORD_ID_MAX 16

enum GP_MetaType {
	GP_META_INT,
	GP_META_STRING,
	GP_META_DOUBLE,
};

union GP_MetaValue {
	int i;
	double d;
	const char *str;
};

typedef struct GP_MetaRecord {
	char id[GP_META_RECORD_ID_MAX];
	unsigned int hash;
	enum GP_MetaType type;
	struct GP_MetaRecord *next;
	union GP_MetaValue val;
} GP_MetaRecord;

typedef struct GP_MetaData GP_MetaData;

/*
 * Creates a metadata storage for at least expected_records values.
 *
 * Returns NULL if allocation has failed.
 */
GP_MetaData *GP_MetaDataCreate(unsigned int expected_records);

/*
 * Clears meta-data storage.
 */
void GP_MetaDataClear(GP_MetaData *self);

/*
 * Destroys metadata (frees all alocated memory).
 */
void GP_MetaDataDestroy(GP_MetaData *self);

/*
 * Prints metadata into the stdout.
 */
void GP_MetaDataPrint(GP_MetaData *self);

/*
 * Looks for metadata record with id.
 */
GP_MetaRecord *GP_MetaDataGetRecord(GP_MetaData *self, const char *id);

/*
 * Looks for integer metadata with id. Returns 0 on success and res is set to
 * found metadata value.
 */
int GP_MetaDataGetInt(GP_MetaData *self, const char *id, int *res);

/*
 * Looks for string metadata by id. Returns pointe to found string, or NULL if
 * there was no such value.
 */
const char *GP_MetaDataGetString(GP_MetaData *self, const char *id);

/*
 * Creates an record and returns pointer to it.
 *
 * May return NULL if allocation has failed.
 */
GP_MetaRecord *GP_MetaDataCreateRecord(GP_MetaData *self, const char *id);

/*
 * Creates an integer record and returns pointer to it.
 */
GP_MetaRecord *GP_MetaDataCreateInt(GP_MetaData *self, const char *id, int val);

/*
 * Creates an double record and returns pointer to it.
 */
GP_MetaRecord *GP_MetaDataCreateDouble(GP_MetaData *self, const char *id,
                                       double val);

/*
 * Creates an string record and returns pointer to it.
 *
 * If dup is set to 1, the string is duplicated inside of the MetaData
 * structure, otherwise only the pointer is saved.
 */
GP_MetaRecord *GP_MetaDataCreateString(GP_MetaData *self, const char *id,
                                       const char *str, int dup);

#endif /* LOADERS_GP_METADATA_H */
