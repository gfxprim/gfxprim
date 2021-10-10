//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIR_CACHE_H
#define GP_DIR_CACHE_H

#include <time.h>

typedef struct gp_dir_entry {
	size_t size;
	time_t mtime;
	unsigned int name_len;
	int is_dir:1;
	int filtered:1;
	char name[];
} gp_dir_entry;

typedef struct gp_dir_cache {
	DIR *dir;
	int dirfd;
	int inotify_fd;
	int sort_type;
	struct gp_block *allocator;
	size_t filtered;
	size_t size;
	size_t used;
	struct gp_dir_entry **entries;
} gp_dir_cache;

gp_dir_cache *gp_dir_cache_new(const char *path);

void gp_dir_cache_free(gp_dir_cache *self);

enum gp_dir_cache_sort_type {
	GP_DIR_SORT_ASC = 0x00,
	GP_DIR_SORT_DESC = 0x04,
	GP_DIR_SORT_BY_NAME = 0x00,
	GP_DIR_SORT_BY_SIZE = 0x01,
	GP_DIR_SORT_BY_MTIME = 0x02,
};

void gp_dir_cache_sort(gp_dir_cache *self, int sort_type);

static inline gp_dir_entry *gp_dir_cache_get(gp_dir_cache *self,
                                             unsigned int pos)
{
	if (self->used <= pos)
		return NULL;

	return self->entries[pos];
}

/*
 * If element has been set to be filtered it's ignored by functions with _filter suffix.
 *
 * @self Directory cache.
 * @pos Element position
 * @filter Either 1 == filtered or 0 == not filtered.
 */
static inline void gp_dir_cache_set_filter(gp_dir_cache *self, unsigned int pos,
                                           int filter)
{
	if (self->entries[pos]->filtered == !!filter)
		return;

	self->entries[pos]->filtered = !!filter;
	self->filtered += filter ? 1 : -1;
}

/*
 * @brief Returns number of entries.
 *
 * @return A number of entries.
 */
static inline size_t gp_dir_cache_entries(gp_dir_cache *self)
{
	return self->used;
}

/*
 * @brief Returns number of not-filtered entries.
 *
 * @return A number of entries.
 */
static inline size_t gp_dir_cache_entries_filter(gp_dir_cache *self)
{
	return self->used - self->filtered;
}

/*
 * Returns entry on position pos ignoring filtered out elements.
 *
 * @self A dir cache.
 * @pos position of the element
 * @return A dir cache entry or NULL if position is not occupied.
 */
gp_dir_entry *gp_dir_cache_get_filtered(gp_dir_cache *self, unsigned int pos);

/*
 * Inotify handler, should be called when there are data to be read on inotify_fd.
 *
 * @self struct gp_dir_cache
 * @return Returns non-zeor if cache content changed.
 */
int gp_dir_cache_inotify(gp_dir_cache *self);

/*
 * Request update when directory was created in the cache directory.
 *
 * @self A dir cache.
 * @dirname A name of a directory that was created.
 */
void gp_dir_cache_new_dir(gp_dir_cache *self, const char *dirname);

/**
 * Looks up for a file in the cache and returns an position. Note that the
 * position is not stable and will change on sort!
 *
 * @self A dir cache.
 * @name A directory or file name.
 */
unsigned int gp_dir_cache_pos_by_name_filtered(gp_dir_cache *self, const char *name);

enum gp_dir_cache_type {
	GP_DIR_CACHE_NONE = 0,
	GP_DIR_CACHE_FILE = 1,
	GP_DIR_CACHE_DIR = 2,
};

/**
 * Looks for a file in the directory the cache operates in.
 *
 * @self A dir cache.
 * @name A filename.
 */
enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *self, const char *name);

#endif /* GP_DIR_CACHE_H */
