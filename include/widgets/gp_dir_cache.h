//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_DIR_CACHE_H
#define GP_DIR_CACHE_H

#include <utils/gp_poll.h>
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
	int sort_type;
	struct gp_balloc_pool *allocator;
	size_t filtered;
	size_t size;
	size_t used;
	struct gp_dir_entry **entries;
} gp_dir_cache;

/**
 * Creates and populates a new directory cache
 *
 * @path A path to allocate the cache for
 * @return Newly allocated and populated directory cache or NULL in case of
 *         allocation failure
 */
gp_dir_cache *gp_dir_cache_new(const char *path);

/**
 * Destroy a directory cache.
 *
 * @self A directory cache to destroy.
 */
void gp_dir_cache_destroy(gp_dir_cache *self);

/**
 * @brief Adds an entry to the directory cache.
 *
 * This function is called by the platform code.
 *
 * @self A directory cache to add the entry to
 * @size A file size in bytes
 * @name A file name
 * @mode A file mode
 * @mtime A modification timestamp
 */
gp_dir_entry *gp_dir_cache_add_entry(gp_dir_cache *self, size_t size,
                                     const char *name, mode_t mode, time_t mtime);

/**
 * @brief Revoves an entry from directory cache.
 *
 * This function is called by the platform code.
 *
 * @self A directory cache to remove the entry from
 * @name An entry name to be removed
 */
int gp_dir_cache_rem_entry_by_name(gp_dir_cache *self, const char *name);

/**
 * @brief Looks up an entry based on a file name
 *
 * @self A directory cache
 * @name An entry name to look for
 * @return An directory entry on NULL if there is no such entry
 */
gp_dir_entry *gp_dir_cache_entry_lookup(gp_dir_cache *self, const char *name);

/**
 * @brief Returns true if there is at least one entry with needle in the name
 *
 * @return Non-zero if entry with needle in name exists zero otherwise.
 */
int gp_dir_cache_entry_name_contains(gp_dir_cache *self, const char *needle);

/**
 * @brief Frees all entries from directory cache.
 *
 * This function is called by the platform code.
 *
 * @self A directory cache.
 */
void gp_dir_cache_free_entries(gp_dir_cache *self);

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

/**
 * @brief A change notify handler
 *
 * This function should be called to update the cache content when there are
 * data to be read on inotify_fd.
 *
 * @self A directory cache
 * @return Returns non-zero if cache content changed
 */
int gp_dir_cache_notify(gp_dir_cache *self);

/**
 * @brief Returns notify_fd if available.
 *
 * @self A direcotry cache
 * @return A file descriptor structure for gp_poll().
 */
gp_fd *gp_dir_cache_notify_fd(gp_dir_cache *self);

/**
 * @brief Creates a directory and updates the cache.
 *
 * @self A dir cache.
 * @dirname A directory name.
 * @return Returns an errno on a failure.
 */
int gp_dir_cache_mkdir(gp_dir_cache *self, const char *dirname);

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
 * @brief Looks for a file in the directory the cache operates in.
 *
 * @self A dir cache.
 * @name A filename.
 */
enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *self, const char *name);

#endif /* GP_DIR_CACHE_H */
