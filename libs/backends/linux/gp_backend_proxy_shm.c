//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2024 Cyril Hrubis <metan@ucw.cz>

 */

#define _GNU_SOURCE
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <core/gp_debug.h>
#include <backends/gp_proxy_shm.h>

static size_t round_to_page_size(size_t size)
{
	static size_t page_mask;

	if (!page_mask)
		page_mask = getpagesize() - 1;

	size_t ret = (size + page_mask) & (~page_mask);

	GP_DEBUG(2, "Rounding %zu to %zu", size, ret);

	return ret;
}

gp_proxy_shm *gp_proxy_shm_init(const char *path, gp_size w, gp_size h, gp_pixel_type type)
{
	size_t path_size = strlen(path)+1;

	if (path_size >= 64) {
		GP_WARN("SHM path too long!");
		return NULL;
	}

	gp_proxy_shm *ret = malloc(sizeof(struct gp_proxy_shm));

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	strcpy(ret->path.path, path);

	gp_pixmap_init(&ret->pixmap, w, h, type, NULL, 0);

	size_t size = round_to_page_size(ret->pixmap.bytes_per_row * h);

	unlink(path);

	int fd = open(path, O_EXCL | O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		GP_WARN("Failed to open SHMEM: %s", strerror(errno));
		goto err0;
	}

	if (ftruncate(fd, size)) {
		GP_WARN("ftruncate() failed: %s", strerror(errno));
		goto err1;
	}

	void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		GP_WARN("mmap() failed: %s", strerror(errno));
		goto err1;
	}

	ret->pixmap.pixels = p;
	ret->size = size;
	ret->fd = fd;

	ret->path.size = size;

	return ret;
err1:
	close(fd);
	unlink(path);
err0:
	free(ret);
	return NULL;
}

int gp_proxy_shm_resize(gp_proxy_shm *self, gp_size w, gp_size h)
{
	gp_pixmap new;
	gp_pixel_type ptype = self->pixmap.pixel_type;

	gp_pixmap_init(&new, w, h, ptype, NULL, 0);

	size_t new_size = round_to_page_size(new.bytes_per_row * h);

	if (self->size == new_size) {
		gp_pixmap_init(&self->pixmap, w, h, ptype, self->pixmap.pixels, 0);
		return 0;
	}

	if (ftruncate(self->fd, new_size)) {
		GP_WARN("ftruncate() failed: %s", strerror(errno));
		return -1;
	}

	void *p = mremap(self->pixmap.pixels, self->size, new_size, MREMAP_MAYMOVE, NULL);

	if (!p) {
		GP_WARN("mremap() failed: %s", strerror(errno));
		return -1;
	}

	GP_DEBUG(1, "remapped buffer to %zu bytes", new_size);

	self->size = new_size;
	self->path.size = new_size;
	gp_pixmap_init(&self->pixmap, w, h, ptype, p, 0);
	return 1;
}

void gp_proxy_shm_exit(gp_proxy_shm *self)
{
	close(self->fd);
	munmap(self->pixmap.pixels, self->size);
	unlink(self->path.path);
	free(self);
}
