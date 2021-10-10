//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/inotify.h>

#include <core/gp_debug.h>
#include <utils/gp_block_alloc.h>
#include <widgets/gp_dir_cache.h>

static gp_dir_entry *new_entry(gp_dir_cache *self, size_t size,
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

	return entry;
}

static void put_entry(gp_dir_cache *self, gp_dir_entry *entry)
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

static void add_entry(gp_dir_cache *self, const char *name)
{
	gp_dir_entry *entry;
	struct stat buf;

	if (fstatat(self->dirfd, name, &buf, 0)) {
		GP_DEBUG(3, "stat(%s): %s", name, strerror(errno));
		return;
	}

	entry = new_entry(self, buf.st_size, name,
	                  buf.st_mode, buf.st_mtim.tv_sec);
	if (!entry)
		return;

	put_entry(self, entry);
}

static int rem_entry_by_name(gp_dir_cache *self, const char *name)
{
	unsigned int i;

	for (i = 0; i < self->used; i++) {
		if (!strcmp(self->entries[i]->name, name)) {
			self->entries[i] = self->entries[--self->used];
			return 0;
		}
	}

	return 1;
}

static void populate(gp_dir_cache *self)
{
	for (;;) {
		struct dirent *ent;

		ent = readdir(self->dir);
		if (!ent)
			return;

		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		add_entry(self, ent->d_name);
	}
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

static void open_inotify(gp_dir_cache *self, const char *path)
{
	self->inotify_fd = inotify_init1(IN_NONBLOCK);

	if (self->inotify_fd < 0) {
		GP_DEBUG(1, "inotify_init(): %s", strerror(errno));
		return;
	}

	int watch = inotify_add_watch(self->inotify_fd, path, IN_CREATE | IN_DELETE);

	if (watch < 0) {
		GP_DEBUG(1, "inotify_add_watch(): %s", strerror(errno));
		close(self->inotify_fd);
		self->inotify_fd = -1;
		return;
	}
}

static void close_inotify(gp_dir_cache *self)
{
	if (self->inotify_fd > 0)
		close(self->inotify_fd);
}

static void append_slash(struct inotify_event *ev)
{
	size_t len = strlen(ev->name);

	if (ev->name[len] == '/')
		return;

	if (len + 1 >= ev->len)
		return;

	ev->name[len] = '/';
	ev->name[len+1] = 0;
}

static int dir_cache_inotify(gp_dir_cache *self, const char *new_dir)
{
	long buf[1024];
	struct inotify_event *ev = (void*)buf;
	int sort = 0;

	if (self->inotify_fd <= 0)
		return 0;

	while (read(self->inotify_fd, &buf, sizeof(buf)) > 0) {
		switch (ev->mask) {
		case IN_DELETE:
			if (!rem_entry_by_name(self, ev->name)) {
				GP_DEBUG(1, "Deleted '%s'", ev->name);
				sort |= 1;
				break;
			}
		/*
		 * We have to try both since symlink to directory does not
		 * contain IN_ISDIR but appears to be directory for us.
		 */
		/* fallthrough */
		case IN_DELETE | IN_ISDIR:
			append_slash(ev);

			GP_DEBUG(1, "Deleted '%s'", ev->name);

			if (rem_entry_by_name(self, ev->name))
				GP_WARN("Failed to remove '%s'", ev->name);

			sort |= 1;
		break;
		case IN_CREATE | IN_ISDIR:
			if (new_dir && !strcmp(new_dir, ev->name))
				sort |= 2;
		/* fallthrough */
		case IN_CREATE:
			GP_DEBUG(1, "Created '%s'", ev->name);
			add_entry(self, ev->name);
			sort |= 1;
		break;
		}
	}

	if (sort)
		gp_dir_cache_sort(self, self->sort_type);

	return sort;
}

int gp_dir_cache_inotify(gp_dir_cache *self)
{
	return dir_cache_inotify(self, NULL);
}

int gp_dir_cache_mkdir(gp_dir_cache *self, const char *dirname)
{
	if (mkdirat(self->dirfd, dirname, 0755))
		return errno;

	/* Make sure the directory is in the case upon fucntion exit */
	if (gp_dir_cache_inotify(self) & 2)
		return 0;

	/* Fallback when inotify is not enabled */
	add_entry(self, dirname);

	return 0;
}

#define MIN_SIZE 25

gp_dir_cache *gp_dir_cache_new(const char *path)
{
	DIR *dir;
	int dirfd;
	gp_dir_cache *ret;
	gp_dir_entry **entries;

	GP_DEBUG(1, "Creating dir cache for '%s'", path);

	ret = malloc(sizeof(gp_dir_cache));
	entries = malloc(MIN_SIZE * sizeof(void*));
	if (!ret || !entries) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	ret->entries = entries;

	open_inotify(ret, path);

	dirfd = open(path, O_DIRECTORY);
	if (!dirfd) {
		GP_DEBUG(1, "open(%s, O_DIRECTORY): %s", path, strerror(errno));
		goto err0;
	}

	dir = opendir(path);
	if (!dir) {
		GP_DEBUG(1, "opendir(%s) failed: %s", path, strerror(errno));
		goto err1;
	}

	ret->dir = dir;
	ret->dirfd = dirfd;
	ret->size = MIN_SIZE;
	ret->used = 0;
	ret->filtered = 0;
	ret->allocator = NULL;
	ret->sort_type = 0;

	//TODO: handle correctly all variants that resolve to "/"
	if (strcmp(path, "/"))
		add_entry(ret, "..");

	populate(ret);

	gp_dir_cache_sort(ret, ret->sort_type);

	return ret;
err1:
	close(dirfd);
err0:
	close_inotify(ret);
	free(ret->entries);
	free(ret);
	return NULL;
}

void gp_dir_cache_free(gp_dir_cache *self)
{
	GP_DEBUG(1, "Destroying dir cache %p", self);

	close_inotify(self);

	closedir(self->dir);
	close(self->dirfd);
	gp_block_free(&self->allocator);
	free(self->entries);
	free(self);
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

enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *self, const char *name)
{
	struct stat buf;

	if (fstatat(self->dirfd, name, &buf, 0)) {
		if (errno != ENOENT)
			GP_DEBUG(3, "stat(%s): %s", name, strerror(errno));

		return GP_DIR_CACHE_NONE;
	}

	if ((buf.st_mode & S_IFMT) == S_IFDIR)
		return GP_DIR_CACHE_DIR;

	return GP_DIR_CACHE_FILE;
}
