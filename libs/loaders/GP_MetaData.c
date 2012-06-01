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

#include <stdlib.h>
#include <string.h>

#include "core/GP_Debug.h"

#include "GP_MetaData.h"

struct GP_MetaData {
	struct GP_MetaRecord *root;
	struct GP_MetaRecord *last;
	unsigned int rec_count;
	size_t size;
	size_t free;
	char buf[];
};

static unsigned int do_hash(const char *id)
{
	unsigned int hash = 0;

	while (*id != '\0') {
		hash += *id * 1217;
		hash %= 46309;
		id += 1;
	}

	return hash;
}

GP_MetaData *GP_MetaDataCreate(unsigned int expected_records)
{
	GP_MetaData *data;
	size_t size = expected_records * sizeof(struct GP_MetaRecord);

	GP_DEBUG(1, "Creating MetaData for %u records", expected_records);

	data = malloc(sizeof(struct GP_MetaData) + size);

	if (data == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	data->root = NULL;
	data->last = NULL;
	data->rec_count = 0;
	data->size = size;
	data->free = size;

	return data;
}

void GP_MetaDataClear(GP_MetaData *self)
{
	GP_DEBUG(1, "Clearing MetaData %p with %u records",
	         self, self->rec_count);
	
	self->root = NULL;
	self->last = NULL;
	self->rec_count = 0;
	self->free = self->size;
}

void GP_MetaDataDestroy(GP_MetaData *self)
{
	GP_DEBUG(1, "Destroying MetaData %p", self);
	free(self);
}

void GP_MetaDataPrint(GP_MetaData *self)
{
	GP_MetaRecord *rec;

	printf("MetaData %u record(s)\n", self->rec_count);

	for (rec = self->root; rec != NULL; rec = rec->next) {
		printf("%-32s: ", rec->id);

		switch (rec->type) {
		case GP_META_INT:
			printf("%i\n", rec->val.i);
		break;
		case GP_META_RATIONAL:
			printf("%i/%i\n", rec->val.r.num, rec->val.r.den);
		break;
		case GP_META_STRING:
			printf("'%s'\n", rec->val.str);
		break;
		case GP_META_DOUBLE:
			printf("%lf\n", rec->val.d);
		break;
		}
	}
}

static GP_MetaRecord *record_lookup(GP_MetaData *self, const char *id,
                                    unsigned int hash)
{
	GP_MetaRecord *rec;

	for (rec = self->root; rec != NULL; rec = rec->next)
		if (rec->hash == hash && !strcmp(rec->id, id))
			return rec;
	
	return NULL;
}

static void *do_alloc(struct GP_MetaData *self, size_t size)
{
	if (self->free < size) {
		GP_DEBUG(0, "TODO: storage full");
		return NULL;
	}

	void *ret = ((char*)self) + sizeof(struct GP_MetaData) + (self->size - self->free);

	self->free -= size;
	
	return ret;
}

static GP_MetaRecord *record_create(GP_MetaData *self, const char *id,
                                    unsigned int hash)
{
	GP_MetaRecord *rec;
	
	if (strlen(id) + 1 > GP_META_RECORD_ID_MAX) {
		GP_DEBUG(0, "Can't create id '%s' longer than %i chars",
		         id, GP_META_RECORD_ID_MAX - 1);
		return NULL;
	}

	rec = do_alloc(self, sizeof(struct GP_MetaRecord));
	
	if (rec == NULL)
		return NULL;

	strcpy(rec->id, id);
	rec->hash = hash;
	
	if (self->root == NULL) {
		self->root = rec;
		self->last = rec;
	} else {
		self->last->next = rec;
		self->last = rec;
		rec->next = NULL;
	}
	
	self->rec_count++;

	return rec;
}

GP_MetaRecord *GP_MetaDataCreateRecord(GP_MetaData *self, const char *id)
{
	unsigned int hash = do_hash(id);

	if (record_lookup(self, id, hash)) {
		GP_DEBUG(1, "Trying to create duplicate record id '%s'", id);
		return NULL;
	}

	return record_create(self, id, hash);	
}

int GP_MetaDataGetInt(GP_MetaData *self, const char *id, int *res)
{
	GP_MetaRecord *rec;
	
	GP_DEBUG(2, "Looking for GP_META_INT id '%s'", id);
	
	rec = record_lookup(self, id, do_hash(id));

	if (rec == NULL) {
		GP_DEBUG(3, "Record id '%s' not found", id);
		return 1;
	}

	if (rec->type != GP_META_INT) {
		GP_DEBUG(3, "Record id '%s' has wrong type", id);
		return 1;
	}

	*res = rec->val.i;

	GP_DEBUG(3, "Found GP_META_INT id '%s' = %i", id, *res);

	return 0;
}

int GP_MetaDataGetDouble(GP_MetaData *self, const char *id, double *res)
{
	GP_MetaRecord *rec;
	
	GP_DEBUG(2, "Looking for GP_META_DOUBLE id '%s'", id);
	
	rec = record_lookup(self, id, do_hash(id));

	if (rec == NULL) {
		GP_DEBUG(3, "Record id '%s' not found", id);
		return 1;
	}

	if (rec->type != GP_META_DOUBLE) {
		GP_DEBUG(3, "Record id '%s' has wrong type", id);
		return 1;
	}

	*res = rec->val.d;

	GP_DEBUG(3, "Found GP_META_DOUBLE id '%s' = %lf", id, *res);

	return 0;
}

const char *GP_MetaDataGetString(GP_MetaData *self, const char *id)
{
	GP_MetaRecord *rec;
	
	GP_DEBUG(2, "Looking for GP_META_STRING id '%s'", id);
	
	rec = record_lookup(self, id, do_hash(id));

	if (rec == NULL) {
		GP_DEBUG(3, "Record id '%s' not found", id);
		return NULL;
	}

	if (rec->type != GP_META_STRING) {
		GP_DEBUG(3, "Record id '%s' has wrong type", id);
		return NULL;
	}

	GP_DEBUG(3, "Found GP_META_STRING id '%s' = '%s'", id, rec->val.str);

	return rec->val.str;
}

GP_MetaRecord *GP_MetaDataCreateInt(GP_MetaData *self, const char *id, int val)
{
	GP_MetaRecord *rec;

	GP_DEBUG(2, "Creating GP_META_INT id '%s' = %i", id, val);
	
	rec = GP_MetaDataCreateRecord(self, id);

	if (rec == NULL)
		return NULL;
	
	rec->type = GP_META_INT;
	rec->val.i = val;

	return rec;
}

GP_MetaRecord *GP_MetaDataCreateRat(GP_MetaData *self, const char *id,
                                    int num, int den)
{
	GP_MetaRecord *rec;

	GP_DEBUG(2, "Creating GP_META_RATIONAL id '%s' = %i/%i", id, num, den);
	
	if (den == 0) {
		GP_DEBUG(1, "Would not create '%s' with denominator == 0", id);
		return NULL;
	}

	rec = GP_MetaDataCreateRecord(self, id);

	if (rec == NULL)
		return NULL;
	
	rec->type = GP_META_RATIONAL;
	rec->val.r.num = num;
	rec->val.r.den = den;

	return rec;
}

GP_MetaRecord *GP_MetaDataCreateDouble(GP_MetaData *self, const char *id,
                                       double val)
{
	GP_MetaRecord *rec;

	GP_DEBUG(2, "Creating GP_META_DOUBLE id '%s' = %lf", id, val);
	
	rec = GP_MetaDataCreateRecord(self, id);

	if (rec == NULL)
		return NULL;
	
	rec->type = GP_META_DOUBLE;
	rec->val.d = val;

	return rec;
}

GP_MetaRecord *GP_MetaDataCreateString(GP_MetaData *self, const char *id,
                                       const char *str, int len, int dup)
{
	GP_MetaRecord *rec;

	GP_DEBUG(2, "Creating GP_META_STRING id '%s' = '%s'", id, str);

	rec = GP_MetaDataCreateRecord(self, id);

	if (rec == NULL)
		return NULL;

	if (dup) {
		size_t size;
		char *s;
		
		if (len == 0)
			size = strlen(str) + 1;
		else
			size = len + 1;

		/* Play safe with aligment */
		if (size % 8)
			size += 8 - size % 8;

		//TODO: allocation error
		s = do_alloc(self, size);
		strncpy(s, str, size - 1);
		s[size - 1] = '\0';
		str = s;
	}

	rec->type = GP_META_STRING;
	rec->val.str = str;

	return rec;
}
