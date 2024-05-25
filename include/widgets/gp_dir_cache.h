//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_dir_cache.h
 * @brief A cache for a directory content.
 *
 * This implements an data structure to cache a directory content so that it
 * can be listed in an alphabetical order. It also listens for a inotify events
 * so the list is updated whenever the directory content changes.
 */

#ifndef GP_DIR_CACHE_H
#define GP_DIR_CACHE_H

#include <utils/gp_poll.h>
#include <time.h>

/** @brief A dir cache entry, represents a file or a directory */
typedef struct gp_dir_entry {
	/** @brief Entry size in bytes. */
	size_t size;
	/** @brief Entry modification time. */
	time_t mtime;
	/** @brief Length of the entry name. */
	unsigned int name_len;
	/** @brief Set if entry is a directory. */
	int is_dir:1;
	/** @brief If set the entry is hidden from listing. */
	int filtered:1;
	/** @brief Entry name. */
	char name[];
} gp_dir_entry;

/** @brief How should be the listing sorted. */
typedef enum gp_dir_cache_sort_type {
	/** @brief Sort in an ascending order. */
	GP_DIR_SORT_ASC = 0x00,
	/** @brief Sort in a descending order. */
	GP_DIR_SORT_DESC = 0x04,
	/** @brief Sort by name. */
	GP_DIR_SORT_BY_NAME = 0x00,
	/** @brief Sort by size. */
	GP_DIR_SORT_BY_SIZE = 0x01,
	/** @brief Sort by modification time. */
	GP_DIR_SORT_BY_MTIME = 0x02,
} gp_dir_cache_sort_type;

/** @brief A directory cache. */
typedef struct gp_dir_cache {
	/** @brief An order the listing is sorted in. */
	enum gp_dir_cache_sort_type sort_type;
	/** @brief A block allocator for the cache entries. */
	struct gp_balloc_pool *allocator;
	/** @brief Number of filtered items */
	size_t filtered;
	/** @brief The size of the cache. */
	size_t size;
	/** @brief Number of used entries. */
	size_t used;
	/** @brief An array of dir cache entres sorted accordingly to sort_type. */
	gp_dir_entry **entries;
} gp_dir_cache;

/**
 * @brief Creates and populates a new directory cache.
 *
 * Creates a directory cache and fills it with list of entries at the given
 * path. If supported the directory is also set up to with an inotify watch in
 * order to update the listing whenever the directory content changes.
 *
 * @param path A path to load the cache entries from.
 *
 * @return Newly allocated and populated directory cache or NULL in case of
 *         allocation failure.
 */
gp_dir_cache *gp_dir_cache_new(const char *path);

/**
 * @brief Destroys a directory cache.
 *
 * Closes all file descriptors, frees memory.
 *
 * @param self A directory cache to destroy.
 */
void gp_dir_cache_destroy(gp_dir_cache *self);

/**
 * @brief Adds an entry to the directory cache.
 *
 * This function is called by the platform code.
 *
 * @param self A directory cache to add the entry to
 * @param size A file size in bytes
 * @param name A file name
 * @param mode A file mode
 * @param mtime A modification timestamp
 *
 * @return A pointer to a newly allocated directory entry.
 */
gp_dir_entry *gp_dir_cache_add_entry(gp_dir_cache *self, size_t size,
                                     const char *name, mode_t mode, time_t mtime);

/**
 * @brief Removes an entry from directory cache.
 *
 * This function is called by the platform code.
 *
 * @param self A directory cache to remove the entry from
 * @param name An entry name to be removed
 *
 * @return Zero on success i.e. entry was found, non-zero otherwise.
 */
int gp_dir_cache_rem_entry_by_name(gp_dir_cache *self, const char *name);

/**
 * @brief Looks up an entry based on a file name
 *
 * @param self A directory cache.
 * @param name An entry name to look for.
 *
 * @return An directory entry on NULL if there is no such entry
 */
gp_dir_entry *gp_dir_cache_entry_lookup(gp_dir_cache *self, const char *name);

/**
 * @brief Returns true if there is at least one entry with needle in the name
 *
 * @param self A directory cache.
 * @param needle A (sub)string in the name to look for.
 *
 * @return Non-zero if entry with needle in name exists zero otherwise.
 */
int gp_dir_cache_entry_name_contains(gp_dir_cache *self, const char *needle);

/**
 * @brief Frees all entries from directory cache.
 *
 * This function is called by the platform code.
 *
 * @param self A directory cache.
 */
void gp_dir_cache_free_entries(gp_dir_cache *self);

/**
 * @brief Sorts the directory cache entries.
 *
 * @param self A directory cache.
 * @param sort_type A requested sort order.
 */
void gp_dir_cache_sort(gp_dir_cache *self, gp_dir_cache_sort_type sort_type);

/**
 * @brief Returns an entry given a position.
 *
 * @param self A directory cache.
 * @param pos Element position in the gp_dir_cache::entries array.
 *
 * @return A pointer to an entry or NULL if pos is outside of the array.
 */
static inline gp_dir_entry *gp_dir_cache_get(gp_dir_cache *self,
                                             unsigned int pos)
{
	if (self->used <= pos)
		return NULL;

	return self->entries[pos];
}

/**
 * @brief Sets dir cache entry filter flag.
 *
 * If element has been set to be filtered it's ignored by functions with
 * the _filter suffix.
 *
 * @param self A directory cache.
 * @param pos Element position in the gp_dir_cache::entries array.
 * @param filter Either 1 == filtered or 0 == not filtered.
 */
static inline void gp_dir_cache_set_filter(gp_dir_cache *self, unsigned int pos,
                                           int filter)
{
	if (self->entries[pos]->filtered == !!filter)
		return;

	self->entries[pos]->filtered = !!filter;
	self->filtered += filter ? 1 : -1;
}

/**
 * @brief Returns number of entries.
 *
 * @param self A directory cache.
 *
 * @return A number of entries.
 */
static inline size_t gp_dir_cache_entries(gp_dir_cache *self)
{
	return self->used;
}

/**
 * @brief Returns number of not-filtered entries.
 *
 * @param self A directory cache.
 *
 * @return A number of entries.
 */
static inline size_t gp_dir_cache_entries_filter(gp_dir_cache *self)
{
	return self->used - self->filtered;
}

/**
 * @brief Returns entry on position pos ignoring filtered out elements.
 *
 * @param self A directory cache.
 * @param pos Element position in the gp_dir_cache::entries array.
 *
 * @return A dir cache entry or NULL if position is not occupied.
 */
gp_dir_entry *gp_dir_cache_get_filtered(gp_dir_cache *self, unsigned int pos);

/**
 * @brief A change notify handler
 *
 * This function should be called to update the cache content when there are
 * data to be read on inotify fd.
 *
 * @param self A directory cache.
 *
 * @return Returns non-zero if cache content changed.
 */
int gp_dir_cache_notify(gp_dir_cache *self);

/**
 * @brief Returns inotify fd if available.
 *
 * @param self A direcotry cache.
 *
 * @return A file descriptor structure for gp_poll().
 */
gp_fd *gp_dir_cache_notify_fd(gp_dir_cache *self);

/**
 * @brief Creates a directory and updates the cache.
 *
 * @param self A dir cache.
 * @param dirname A directory name.
 *
 * @return Returns an errno on a failure.
 */
int gp_dir_cache_mkdir(gp_dir_cache *self, const char *dirname);

/**
 * @brief Looks up a entry position by name.
 *
 * @warning Looks up for a file in the cache and returns an position. Note that
 *          the position is not stable and will change on sort or when notify
 *          events are processed.
 *
 * @param self A dir cache.
 * @param name A directory or file name.
 *
 * @return A position in the gp_dir_cache::entries array.
 */
unsigned int gp_dir_cache_pos_by_name_filtered(gp_dir_cache *self, const char *name);

/** @brief A cache entry lookup type. */
enum gp_dir_cache_type {
	/** @brief No element found. */
	GP_DIR_CACHE_NONE = 0,
	/** @brief File type element found. */
	GP_DIR_CACHE_FILE = 1,
	/** @brief A directory type element found. */
	GP_DIR_CACHE_DIR = 2,
};

/**
 * @brief Looks for a file in the directory the cache operates in.
 *
 * @param self A dir cache.
 * @param name A filename.
 *
 * @return An entry type, returns GP_DIR_CACHE_NONE if element with such name
 *         wasn't found.
 */
enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *self, const char *name);

#endif /* GP_DIR_CACHE_H */
