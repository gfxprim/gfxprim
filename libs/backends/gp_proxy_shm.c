//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <core/gp_debug.h>
#include <backends/gp_proxy_shm.h>

struct gp_proxy_shm *gp_proxy_shm_init(const char *path, gp_size w, gp_size h, gp_pixel_type type)
{
	size_t path_size = strlen(path)+1;

	if (path_size >= 64) {
		GP_WARN("SHM path too long!");
		return NULL;
	}

	struct gp_proxy_shm *ret = malloc(sizeof(struct gp_proxy_shm));

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	strcpy(ret->path.path, path);

	gp_pixmap_init(&ret->pixmap, w, h, type, NULL);

	size_t size = ret->pixmap.bytes_per_row * h;

	unlink(path);

	int fd = open(path, O_EXCL | O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		GP_WARN("Failed to open SHMEM: %s", strerror(errno));
		goto err0;
	}

	if (ftruncate(fd, size)) {
		GP_WARN("Truncate failed: %s", strerror(errno));
		goto err1;
	}

	void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (p == MAP_FAILED) {
		GP_WARN("Map failed: %s", strerror(errno));
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

int gp_proxy_shm_resize(struct gp_proxy_shm *self, gp_size w, gp_size h)
{
}

void gp_proxy_shm_exit(struct gp_proxy_shm *self)
{
	close(self->fd);
	munmap(self->pixmap.pixels, self->size);
	unlink(self->path.path);
	free(self);
}


