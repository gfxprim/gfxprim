//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIR_CACHE_H__
#define GP_DIR_CACHE_H__

#include <time.h>

typedef struct gp_dir_entry {
	size_t size;
	time_t mtime;
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
	self->entries[pos]->filtered = !!filter;
}

/*
 * Returns entry on position pos ignoring filtered out elements.
 *
 * @self struct gp_dir_cache
 * @pos position of the element
 */
gp_dir_entry *gp_dir_cache_get_filtered(gp_dir_cache *self, unsigned int pos);


/*
 * Inotify handler, should be called when there are data to be read on inotify_fd.
 *
 * @self struct gp_dir_cache
 * @return Returns non-zeor if cache content changed.
 */
int gp_dir_cache_inotify(gp_dir_cache *self);

#endif /* GP_DIR_CACHE_H__ */
