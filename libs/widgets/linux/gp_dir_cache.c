//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

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

#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <utils/gp_block_alloc.h>
#include <widgets/gp_dir_cache.h>

typedef struct gp_dir_cache_linux {
	gp_dir_cache dir_cache;
	DIR *dir;
	int dirfd;
	int inotify_fd;
} gp_dir_cache_linux;

static void add_entry(gp_dir_cache_linux *self, const char *name, int mode)
{
	struct stat buf;

	if (fstatat(self->dirfd, name, &buf, 0)) {
		/*
		 * If file was deleted right after it was created, add a dummy
		 * entry so that we delete it on next inotify event
		 */
		if (errno == ENOENT && mode)
			gp_dir_cache_add_entry(&self->dir_cache, 0, name, mode, 0);
		else
			GP_DEBUG(3, "stat(%s): %s", name, strerror(errno));

		return;
	}

	gp_dir_cache_add_entry(&self->dir_cache, buf.st_size, name,
	                       buf.st_mode, buf.st_mtim.tv_sec);
}

static void populate(gp_dir_cache_linux *self)
{
	for (;;) {
		struct dirent *ent;

		ent = readdir(self->dir);
		if (!ent)
			return;

		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		add_entry(self, ent->d_name, 0);
	}
}

static void open_inotify(gp_dir_cache_linux *self, const char *path)
{
	self->inotify_fd = inotify_init1(IN_NONBLOCK);

	if (self->inotify_fd < 0) {
		GP_DEBUG(1, "inotify_init(): %s", strerror(errno));
		return;
	}

	int watch = inotify_add_watch(self->inotify_fd, path, IN_CREATE | IN_DELETE | IN_MOVE);

	if (watch < 0) {
		GP_DEBUG(1, "inotify_add_watch(): %s", strerror(errno));
		close(self->inotify_fd);
		self->inotify_fd = -1;
		return;
	}
}

static void close_inotify(gp_dir_cache_linux *self)
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

static int parse_inotify_event(gp_dir_cache_linux *self, const char *new_dir, struct inotify_event *ev)
{
	GP_DEBUG(3, "EV MASK %x NAME '%s'", ev->mask, ev->name);

	switch (ev->mask) {
	case IN_MOVED_FROM:
	case IN_DELETE:
		if (!gp_dir_cache_rem_entry_by_name(&self->dir_cache, ev->name)) {
			GP_DEBUG(1, "Deleted '%s'", ev->name);
			return 1;
			break;
		}
	/*
	 * We have to try both since symlink to directory does not
	 * contain IN_ISDIR but appears to be directory for us.
	 */
	/* fallthrough */
	case IN_MOVED_FROM | IN_ISDIR:
	case IN_DELETE | IN_ISDIR:
		append_slash(ev);

		GP_DEBUG(1, "Deleted dir '%s'", ev->name);

		if (gp_dir_cache_rem_entry_by_name(&self->dir_cache, ev->name))
			GP_WARN("Failed to remove '%s'", ev->name);

		return 1;
	break;
	case IN_MOVED_TO | IN_ISDIR:
	case IN_CREATE | IN_ISDIR:
		if (new_dir && !strcmp(new_dir, ev->name))
			return 2;

		GP_DEBUG(1, "Created dir '%s'", ev->name);
		add_entry(self, ev->name, S_IFDIR);
	break;
	case IN_MOVED_TO:
	case IN_CREATE:
		GP_DEBUG(1, "Created '%s'", ev->name);
		add_entry(self, ev->name, S_IFREG);
		return 1;
	break;
	}

	return 0;
}

static int dir_cache_inotify(gp_dir_cache_linux *self, const char *new_dir)
{
	char buf[2048];
	int sort = 0;
	ssize_t len;

	if (self->inotify_fd <= 0)
		return 0;

	while ((len = read(self->inotify_fd, &buf, sizeof(buf))) > 0) {
		ssize_t i = 0;

		while (i < len) {
			struct inotify_event *ev = (void*)(buf+i);

			sort |= parse_inotify_event(self, new_dir, ev);

			i += sizeof(struct inotify_event) + ev->len;
		}
	}

	if (sort)
		gp_dir_cache_sort(&self->dir_cache, self->dir_cache.sort_type);

	return sort;
}

int gp_dir_cache_notify(gp_dir_cache *cache)
{
	gp_dir_cache_linux *self = GP_CONTAINER_OF(cache, gp_dir_cache_linux, dir_cache);

	return dir_cache_inotify(self, NULL);
}

int gp_dir_cache_notify_fd(gp_dir_cache *cache)
{
	gp_dir_cache_linux *self = GP_CONTAINER_OF(cache, gp_dir_cache_linux, dir_cache);

	return self->inotify_fd;
}

int gp_dir_cache_mkdir(gp_dir_cache *cache, const char *dirname)
{
	gp_dir_cache_linux *self = GP_CONTAINER_OF(cache, gp_dir_cache_linux, dir_cache);

	if (mkdirat(self->dirfd, dirname, 0755))
		return errno;

	/* Make sure the directory is in the case upon fucntion exit */
	if (gp_dir_cache_notify(cache) & 2)
		return 0;

	/* Fallback when inotify is not enabled */
	add_entry(self, dirname, 0);

	return 0;
}

gp_dir_cache *gp_dir_cache_new(const char *path)
{
	gp_dir_cache_linux *cache;

	GP_DEBUG(1, "Creating dir cache for '%s'", path);

	cache = malloc(sizeof(gp_dir_cache_linux));
	if (!cache) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	memset(cache, 0, sizeof(*cache));

	open_inotify(cache, path);

	cache->dirfd = open(path, O_DIRECTORY);
	if (!cache->dirfd) {
		GP_DEBUG(1, "open(%s, O_DIRECTORY): %s", path, strerror(errno));
		goto err0;
	}

	cache->dir = opendir(path);
	if (!cache->dir) {
		GP_DEBUG(1, "opendir(%s) failed: %s", path, strerror(errno));
		goto err1;
	}

	//TODO: handle correctly all variants that resolve to "/"
	if (strcmp(path, "/"))
		add_entry(cache, "..", 0);

	populate(cache);

	gp_dir_cache_sort(&cache->dir_cache, cache->dir_cache.sort_type);

	return &cache->dir_cache;
err1:
	close(cache->dirfd);
err0:
	close_inotify(cache);
	free(cache);
	return NULL;
}

void gp_dir_cache_destroy(gp_dir_cache *cache)
{
	gp_dir_cache_linux *self = GP_CONTAINER_OF(cache, gp_dir_cache_linux, dir_cache);

	GP_DEBUG(1, "Destroying dir cache %p", self);

	close_inotify(self);

	closedir(self->dir);
	close(self->dirfd);
	gp_dir_cache_free_entries(cache);
	free(self);
}

enum gp_dir_cache_type gp_dir_cache_lookup(gp_dir_cache *cache, const char *name)
{
	gp_dir_cache_linux *self = GP_CONTAINER_OF(cache, gp_dir_cache_linux, dir_cache);
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
