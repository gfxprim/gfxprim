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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "core/GP_Debug.h"
#include "core/GP_Common.h"

#include "GP_DataStorage.h"

struct record {
	GP_DataNode node;
	char id[];
};

struct GP_DataDict {
	struct GP_DataNode *first;
	struct GP_DataNode *last;
};

#define BLOCK_SIZE 4096
#define BLOCK_MAX 128

struct block {
	size_t size;
	struct block *next;
	char data[];
};

struct GP_DataStorage {
	/* Root dictionary */
	struct GP_DataNode root;
	struct GP_DataDict dict;

	/* Block allocator */
	struct block *blocks;
	struct block *cur_block;
};

/* Align to four bytes boundary */
static size_t align(size_t size)
{
	size_t mask = 3;

	return (size + mask) & ~mask;
}

static struct block *new_block(GP_DataStorage *self, size_t size)
{
	struct block *new = malloc(size);

	GP_DEBUG(3, "Allocating new block for %zu bytes", size);

	if (!new)
		return NULL;

	new->size = size - sizeof(*new);

	new->next = self->blocks;
	self->blocks = new;

	return new;
}

static void *storage_alloc(GP_DataStorage *self, size_t size)
{
	struct block *new;
	void *ret;

	GP_DEBUG(3, "Allocating %zu bytes", size);

	if (size >= BLOCK_MAX) {
		new = new_block(self, sizeof(*new) + size);

		if (!new)
			return NULL;

		return new->data;
	}

	if (self->cur_block->size < size) {
		new = new_block(self, BLOCK_SIZE);

		if (!new)
			return NULL;

		self->cur_block = new;
	}

	ret = self->cur_block->data + BLOCK_SIZE - self->cur_block->size;
	self->cur_block->size -= size;

	return ret;
}

static struct record *new_record(GP_DataStorage *self, GP_DataNode *node)
{
	size_t id_len = align(strlen(node->id) + 1);
	size_t payload_len = 0;
	struct record *new;
	void *payload;

	switch (node->type) {
	case GP_DATA_STRING:
		payload_len = align(strlen(node->value.str) + 1);
	break;
	case GP_DATA_DICT:
		payload_len = sizeof(struct GP_DataDict);
	break;
	default:
	break;
	}

	new = storage_alloc(self, sizeof(*new) + id_len + payload_len);

	if (!new)
		return NULL;

	strcpy(new->id, node->id);
	new->node.id = new->id;
	new->node.type = node->type;
	new->node.value = node->value;
	new->node.next = NULL;

	payload = ((void*)new) + sizeof(*new) + id_len;

	switch (node->type) {
	case GP_DATA_STRING:
		strcpy(payload, node->value.str);
		new->node.value.str = payload;
	break;
	case GP_DATA_DICT:
		new->node.value.dict = payload;
		new->node.value.dict->first = NULL;
		new->node.value.dict->last = NULL;
	break;
	default:
	break;
	}

	return new;
}

GP_DataStorage *GP_DataStorageCreate(void)
{
	GP_DataStorage *storage = malloc(sizeof(*storage));

	GP_DEBUG(1, "Creating data storage (%p)", storage);

	if (!storage)
		return NULL;

	storage->root.type = GP_DATA_DICT;
	storage->root.value.dict = &storage->dict;
	storage->dict.first = NULL;
	storage->dict.last = NULL;

	storage->blocks = NULL;
	storage->cur_block = new_block(storage, BLOCK_SIZE);

	if (!storage->cur_block) {
		free(storage);
		return NULL;
	}

	return storage;
}

void GP_DataStorageDestroy(GP_DataStorage *self)
{
	struct block *i, *j;

	if (!self)
		return;

	GP_DEBUG(1, "Destroying data storage");

	for (i = self->blocks; i; ) {
		j = i->next;
		free(i);
		i = j;
	}

	free(self);
}

void GP_DataStorageClear(GP_DataStorage *self)
{
	struct block *i, *j;

	GP_DEBUG(1, "Clearing all data in storage");

	/* Clear all but first block */
	for (i = self->blocks->next; i;) {
		j = i->next;
		free(i);
		i = j;
	}

	/* Rest first block */
	self->cur_block = self->blocks;
	self->cur_block->next = NULL;
	//TODO: Store block size!!
	self->cur_block->size = BLOCK_SIZE - sizeof(struct block);

	/* Reset root dict */
	self->dict.first = NULL;
	self->dict.last = NULL;
}

GP_DataNode *GP_DataStorageRoot(GP_DataStorage *self)
{
	return &self->root;
}

GP_DataNode *GP_DataDictFirst(GP_DataNode *node)
{
	return node->value.dict->first;
}

static void dict_add(GP_DataDict *dict, GP_DataNode *node)
{
	if (!dict->last) {
		dict->first = node;
		dict->last = node;
	} else {
		dict->last->next = node;
		dict->last = node;
	}
}

GP_DataNode *GP_DataStorageAdd(GP_DataStorage *self,
                               GP_DataNode *node, GP_DataNode *data)
{
	struct record *rec;

	GP_DEBUG(2, "Adding '%s' to storage (%p)", data->id, self);

	if (node && node->type != GP_DATA_DICT) {
		GP_WARN("Trying to insert data into %s",
		        GP_DataTypeName(node->type));
		return NULL;
	}

	rec = new_record(self, data);

	if (!rec)
		return NULL;

	if (!node)
		node = GP_DataStorageRoot(self);

	dict_add(node->value.dict, &rec->node);

	return &rec->node;
}

GP_DataNode *GP_DataStorageGet(GP_DataStorage *self,
                               GP_DataNode *node,
                               const char *id)
{
	struct GP_DataNode *i;

	if (!node)
		node = GP_DataStorageRoot(self);

	for (i = GP_DataDictFirst(node); i; i = i->next) {
		if (!strcmp(i->id, id))
			return i;
	}

	return NULL;
}

static struct GP_DataNode *lookup(GP_DataNode *node, const char *id,
                                  const int id_len)
{
	struct GP_DataNode *i;

	if (!node)
		return NULL;

	for (i = GP_DataDictFirst(node); i; i = i->next) {
		if (!strncmp(i->id, id, id_len))
			return i;
	}

	return NULL;
}

static struct GP_DataNode *get_by_path(GP_DataNode *node, const char *path)
{
	unsigned int i;

	for (i = 0; path[i] && path[i] != '/'; i++);

	if (!i)
		return node;

	node = lookup(node, path, i);

	if (!node)
		return NULL;

	GP_DEBUG(3, "Lookup has node '%s'", node->id);

	if (path[i] == '/')
		path++;

	path+=i;

	return get_by_path(node, path);
}

GP_DataNode *GP_DataStorageGetByPath(GP_DataStorage *self, GP_DataNode *node,
                                     const char *path)
{
	GP_DEBUG(3, "Looking for '%s' in %p", path, node);

	if (path[0] == '/') {

		if (!self)
			return NULL;

		node = GP_DataStorageRoot(self);
		path++;
	}

	return get_by_path(node, path);
}


static void padd_printf(size_t padd, const char *id, size_t id_padd,
                        const char *fmt, ...)
	__attribute__ ((format (printf, 4, 5)));

static void padd_printf(size_t padd, const char *id, size_t id_padd,
		        const char *fmt, ...)
{
	va_list va;

	while (padd--)
		fputc(' ', stdout);

	if (id)
		fputs(id, stdout);

	if (id_padd)
		id_padd -= strlen(id);

	while (id_padd--)
		fputc(' ', stdout);

	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}

/* Must be called on data dict only */
static size_t max_id_len(const GP_DataNode *node)
{
	size_t max = 0;
	GP_DataNode *i;

	for (i = node->value.dict->first; i; i = i->next)
		max = GP_MAX(max, strlen(i->id));

	return max;
}

static void data_print(const GP_DataNode *node,
		       unsigned int padd, size_t id_padd)
{
	GP_DataNode *i;

	if (!node) {
		padd_printf(padd, NULL, 0, "(Empty)\n");
		return;
	}

	switch (node->type) {
	case GP_DATA_INT:
		padd_printf(padd, node->id, id_padd, " : %li\n", node->value.i);
	break;
	case GP_DATA_DOUBLE:
		padd_printf(padd, node->id, id_padd, " : %lf\n", node->value.d);
	break;
	case GP_DATA_RATIONAL:
		padd_printf(padd, node->id, id_padd, " : %li/%li\n",
		            node->value.rat.num, node->value.rat.den);
	break;
	case GP_DATA_STRING:
		padd_printf(padd, node->id, id_padd, " : '%s'\n", node->value.str);
	break;
	case GP_DATA_DICT:
		padd_printf(padd, node->id ? node->id : "Data Root", 0, " = {\n");

		for (i = node->value.dict->first; i; i = i->next)
			data_print(i, padd + 1, max_id_len(node));

		padd_printf(padd, NULL, 0, "}\n");
	break;
	}
}

void GP_DataPrint(const GP_DataNode *node)
{
	data_print(node, 0, node->id ? strlen(node->id) : 0);
}

const char *GP_DataTypeName(enum GP_DataType type)
{
	switch (type) {
	case GP_DATA_INT:
		return "Int";
	case GP_DATA_DOUBLE:
		return "Double";
	case GP_DATA_RATIONAL:
		return "Rational";
	case GP_DATA_STRING:
		return "String";
	case GP_DATA_DICT:
		return "Dict";
	}

	return "Invalid";
}

GP_DataNode *GP_DataStorageAddInt(GP_DataStorage *self, GP_DataNode *node,
                                  const char *id, long i)
{
	GP_DataNode data = {
		.type = GP_DATA_INT,
		.id = id,
		.value.i = i,
	};

	return GP_DataStorageAdd(self, node, &data);
}

GP_DataNode *GP_DataStorageAddString(GP_DataStorage *self, GP_DataNode *node,
                                     const char *id, const char *str)
{
	GP_DataNode data = {
		.type = GP_DATA_STRING,
		.id = id,
		.value.str = str,
	};

	return GP_DataStorageAdd(self, node, &data);
}

GP_DataNode *GP_DataStorageAddDouble(GP_DataStorage *self, GP_DataNode *node,
                                     const char *id, double d)
{
	GP_DataNode data = {
		.type = GP_DATA_DOUBLE,
		.id = id,
		.value.d = d,
	};

	return GP_DataStorageAdd(self, node, &data);
}

GP_DataNode *GP_DataStorageAddRational(GP_DataStorage *self, GP_DataNode *node,
                                       const char *id, long num, long den)
{
	GP_DataNode data = {
		.type = GP_DATA_RATIONAL,
		.id = id,
		.value.rat = {.num = num, .den = den},
	};

	return GP_DataStorageAdd(self, node, &data);
}

GP_DataNode *GP_DataStorageAddDict(GP_DataStorage *self, GP_DataNode *node,
		                   const char *id)
{
	GP_DataNode data = {
		.type = GP_DATA_DICT,
		.id = id,
	};

	return GP_DataStorageAdd(self, node, &data);
}
