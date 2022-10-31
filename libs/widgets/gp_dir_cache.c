//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <core/gp_debug.h>
#include <utils/gp_block_alloc.h>
#include <widgets/gp_dir_cache.h>

static void add_entry(gp_dir_cache *self, gp_dir_entry *entry)
{
	if (self->used >= self->size) {
		size_t new_size = self->size + 50;
		void *entries;

		entries = realloc(self->entries, new_size * sizeof(void*));
		if (!entries) {
			GP_DEBUG(1, "Realloc failed :-(");
			return;
		}

		self->size = new_size;
		self->entries = entries;
	}

	self->entries[self->used++] = entry;
}

gp_dir_entry *gp_dir_cache_add_entry(gp_dir_cache *self, size_t size,
                                     const char *name, mode_t mode, time_t mtime)
{
	size_t name_len = strlen(name);
	size_t entry_size;
	int is_dir = 0;
	gp_dir_entry *entry;

	if ((mode & S_IFMT) == S_IFDIR)
		is_dir = 1;

	entry_size = sizeof(gp_dir_entry) + name_len + is_dir + 1;

	entry = gp_block_alloc(&self->allocator, entry_size);
	if (!entry)
		return NULL;

	entry->size = size;
	entry->is_dir = is_dir;
	entry->name_len = name_len;
	entry->mtime = mtime;
	sprintf(entry->name, "%s%s", name, is_dir ? "/" : "");

	GP_DEBUG(3, "Dir Cache %p new entry '%s' size %zuB", self, entry->name, size);

	add_entry(self, entry);

	return entry;
}

int gp_dir_cache_rem_entry_by_name(gp_dir_cache *self, const char *name)
{
	size_t i;

	for (i = 0; i < self->used; i++) {
		if (!strcmp(self->entries[i]->name, name)) {
			self->entries[i] = self->entries[--self->used];
			return 0;
		}
	}

	return 1;
}

void gp_dir_cache_free_entries(gp_dir_cache *self)
{
	gp_block_free(&self->allocator);
	free(self->entries);
}

static int cmp_asc_name(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	return strcmp((*ea)->name, (*eb)->name);
}

static int cmp_desc_name(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	return strcmp((*eb)->name, (*ea)->name);
}

static int cmp_asc_size(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	if ((*ea)->size == (*eb)->size)
		return 0;

	return (*ea)->size > (*eb)->size;
}

static int cmp_desc_size(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	if ((*ea)->size == (*eb)->size)
		return 0;

	return (*ea)->size < (*eb)->size;
}

static int cmp_asc_time(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	if ((*ea)->mtime == (*eb)->mtime)
		return 0;

	return (*ea)->mtime > (*eb)->mtime;
}

static int cmp_desc_time(const void *a, const void *b)
{
	const gp_dir_entry *const *ea = a;
	const gp_dir_entry *const *eb = b;

	if ((*ea)->mtime == (*eb)->mtime)
		return 0;

	return (*ea)->mtime < (*eb)->mtime;
}

static int (*cmp_funcs[])(const void *, const void *) = {
	[GP_DIR_SORT_ASC  | GP_DIR_SORT_BY_NAME] = cmp_asc_name,
	[GP_DIR_SORT_DESC | GP_DIR_SORT_BY_NAME] = cmp_desc_name,
	[GP_DIR_SORT_ASC  | GP_DIR_SORT_BY_SIZE] = cmp_asc_size,
	[GP_DIR_SORT_DESC | GP_DIR_SORT_BY_SIZE] = cmp_desc_size,
	[GP_DIR_SORT_ASC  | GP_DIR_SORT_BY_MTIME] = cmp_asc_time,
	[GP_DIR_SORT_DESC | GP_DIR_SORT_BY_MTIME] = cmp_desc_time,
};

void gp_dir_cache_sort(gp_dir_cache *self, int sort_type)
{
	int (*cmp_func)(const void *, const void *) = cmp_funcs[sort_type];

	if (!cmp_func)
		return;

	self->sort_type = sort_type;

	if (strcmp(self->entries[0]->name, "../"))
		qsort(self->entries, self->used, sizeof(void*), cmp_func);
	else
		qsort(self->entries+1, self->used-1, sizeof(void*), cmp_func);
}

gp_dir_entry *gp_dir_cache_get_filtered(gp_dir_cache *self, unsigned int pos)
{
	unsigned int n, cur_pos = 0;

	for (n = 0; n < self->used; n++) {
		if (self->entries[n]->filtered)
			continue;

		if (cur_pos++ == pos)
			return self->entries[n];
	}

	return NULL;
}

unsigned int gp_dir_cache_pos_by_name_filtered(gp_dir_cache *self, const char *name)
{
	unsigned int n, cur_pos = 0;
	size_t len = strlen(name);

	for (n = 0; n < self->used; n++) {
		if (self->entries[n]->filtered)
			continue;

		if (len == self->entries[n]->name_len &&
		    !strncmp(self->entries[n]->name, name, len))
			return cur_pos;

		cur_pos++;
	}

	return (unsigned int)-1;
}

__attribute__((weak))
int gp_dir_cache_notify(gp_dir_cache *self)
{
	(void) self;

	return 0;
}

__attribute__((weak))
int gp_dir_cache_notify_fd(gp_dir_cache *self)
{
	(void) self;

	return -1;
}

__attribute__((weak))
int gp_dir_cache_mkdir(gp_dir_cache *cache, const char *dirname)
{
	(void) cache;
	(void) dirname;

	errno = ENOSYS;
	return -1;
}

__attribute__((weak))
gp_dir_cache *gp_dir_cache_new(const char *path)
{
	(void) path;

	GP_WARN("Stub called");

	errno = ENOSYS;
	return NULL;
}

__attribute__((weak))
void gp_dir_cache_destroy(gp_dir_cache *self)
{
	(void) self;
}

__attribute__((weak))
enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *cache, const char *name)
{
	(void) cache;
	(void) name;

	return 0;
}
