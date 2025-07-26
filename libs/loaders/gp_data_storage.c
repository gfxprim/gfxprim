// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <loaders/gp_data_storage.h>

struct record {
	gp_data_node node;
	char id[];
};

struct gp_data_dict {
	struct gp_data_node *first;
	struct gp_data_node *last;
};

#define BLOCK_SIZE 4096
#define BLOCK_MAX 128

struct block {
	size_t size;
	struct block *next;
	char data[];
};

struct gp_storage {
	/* Root dictionary */
	struct gp_data_node root;
	struct gp_data_dict dict;

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

static struct block *new_block(gp_storage *self, size_t size)
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

static void *storage_alloc(gp_storage *self, size_t size)
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

static struct record *new_record(gp_storage *self, gp_data_node *node)
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
		payload_len = sizeof(struct gp_data_dict);
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

gp_storage *gp_storage_create(void)
{
	gp_storage *storage = malloc(sizeof(*storage));

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

void gp_storage_destroy(gp_storage *self)
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

void gp_storage_clear(gp_storage *self)
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

gp_data_node *gp_storage_root(gp_storage *self)
{
	return &self->root;
}

gp_data_node *gp_data_dict_first(gp_data_node *node)
{
	return node->value.dict->first;
}

static void dict_add(gp_data_dict *dict, gp_data_node *node)
{
	if (!dict->last) {
		dict->first = node;
		dict->last = node;
	} else {
		dict->last->next = node;
		dict->last = node;
	}
}

gp_data_node *gp_storage_add(gp_storage *self,
                             gp_data_node *node, gp_data_node *data)
{
	struct record *rec;
	struct gp_data_node *dup;

	GP_DEBUG(2, "Adding '%s' to storage (%p)", data->id, self);

	if (node && node->type != GP_DATA_DICT) {
		GP_WARN("Trying to insert data into %s",
		        gp_data_type_name(node->type));
		return NULL;
	}

	dup = gp_storage_get(self, node, data->id);

	if (dup) {
		GP_WARN("Trying to insert allready existing node '%s'",
		        data->id);
		return NULL;
	}

	rec = new_record(self, data);

	if (!rec)
		return NULL;

	if (!node)
		node = gp_storage_root(self);

	dict_add(node->value.dict, &rec->node);

	return &rec->node;
}

gp_data_node *gp_storage_get(gp_storage *self,
                             gp_data_node *node,
                             const char *id)
{
	struct gp_data_node *i;

	if (!node)
		node = gp_storage_root(self);

	for (i = gp_data_dict_first(node); i; i = i->next) {
		if (!strcmp(i->id, id))
			return i;
	}

	return NULL;
}

static struct gp_data_node *lookup(gp_data_node *node, const char *id,
                                   const int id_len)
{
	struct gp_data_node *i;

	if (!node)
		return NULL;

	for (i = gp_data_dict_first(node); i; i = i->next) {
		if (!strncmp(i->id, id, id_len))
			return i;
	}

	return NULL;
}

static struct gp_data_node *get_by_path(gp_data_node *node, const char *path)
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

gp_data_node *gp_storage_get_by_path(gp_storage *self, gp_data_node *node,
                                     const char *path)
{
	GP_DEBUG(3, "Looking for '%s' in %p", path, node);

	if (path[0] == '/') {

		if (!self)
			return NULL;

		node = gp_storage_root(self);
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
static size_t max_id_len(const gp_data_node *node)
{
	size_t max = 0;
	gp_data_node *i;

	for (i = node->value.dict->first; i; i = i->next)
		max = GP_MAX(max, strlen(i->id));

	return max;
}

static void data_print(const gp_data_node *node,
		       unsigned int padd, size_t id_padd)
{
	gp_data_node *i;

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

void gp_data_print(const gp_data_node *node)
{
	data_print(node, 0, node->id ? strlen(node->id) : 0);
}

const char *gp_data_type_name(enum gp_data_type type)
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

gp_data_node *gp_storage_add_int(gp_storage *self, gp_data_node *node,
                                 const char *id, long i)
{
	if (!self)
		return NULL;

	gp_data_node data = {
		.type = GP_DATA_INT,
		.id = id,
		.value.i = i,
	};

	return gp_storage_add(self, node, &data);
}

gp_data_node *gp_storage_add_string(gp_storage *self, gp_data_node *node,
                                    const char *id, const char *str)
{
	if (!self)
		return NULL;

	gp_data_node data = {
		.type = GP_DATA_STRING,
		.id = id,
		.value.str = str,
	};

	return gp_storage_add(self, node, &data);
}

gp_data_node *gp_storage_add_double(gp_storage *self, gp_data_node *node,
                                    const char *id, double d)
{
	if (!self)
		return NULL;

	gp_data_node data = {
		.type = GP_DATA_DOUBLE,
		.id = id,
		.value.d = d,
	};

	return gp_storage_add(self, node, &data);
}

gp_data_node *gp_storage_add_rational(gp_storage *self, gp_data_node *node,
                                      const char *id, long num, long den)
{
	if (!self)
		return NULL;

	gp_data_node data = {
		.type = GP_DATA_RATIONAL,
		.id = id,
		.value.rat = {.num = num, .den = den},
	};

	return gp_storage_add(self, node, &data);
}

gp_data_node *gp_storage_add_dict(gp_storage *self, gp_data_node *node,
                                  const char *id)
{
	if (!self)
		return NULL;

	gp_data_node data = {
		.type = GP_DATA_DICT,
		.id = id,
	};

	return gp_storage_add(self, node, &data);
}
