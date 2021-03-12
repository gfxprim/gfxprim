// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>

#include <core/gp_debug.h>
#include <loaders/gp_container.h>
#include <loaders/gp_io.h>
#include <loaders/gp_zip.h>

#define MAX_CONTAINERS 64

static const gp_container_ops *const containers[MAX_CONTAINERS] = {
	&gp_zip_ops,
};

int gp_container_seek(gp_container *self, int offset,
                      enum gp_container_whence whence)
{
	if (!self->ops->seek) {
		GP_DEBUG(1, "Seek not implemented in %s container",
		         self->ops->fmt_name);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->seek(self, offset, whence);
}

int gp_container_load_ex(gp_container *self, gp_pixmap **img,
                         gp_storage *storage, gp_progress_cb *callback)
{
	if (!self->ops->load_ex) {
		GP_DEBUG(1, "Load not implemented in %s container",
		         self->ops->fmt_name);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->load_ex(self, img, storage, callback);
}

const gp_container_ops *gp_container_ops_by_signature(const void *buf)
{
	unsigned int i;

	for (i = 0; containers[i]; i++) {
		if (containers[i]->match && containers[i]->match(buf)) {
			GP_DEBUG(1, "Found container '%s'", containers[i]->fmt_name);
			return containers[i];
		}
	}

	GP_DEBUG(1, "Container not found");
	return NULL;
}

gp_container *gp_container_init(gp_io *io)
{
	char buf[32];
	const gp_container_ops *ops;

	gp_io_mark(io, GP_IO_MARK);

	if (gp_io_fill(io, buf, sizeof(buf))) {
		GP_DEBUG(1, "Failed to read first 32 bytes: %s", strerror(errno));
		return NULL;
	}

	gp_io_mark(io, GP_IO_REWIND);

	ops = gp_container_ops_by_signature(buf);
	if (!ops)
		return NULL;

	if (!ops->init) {
		GP_DEBUG(1, "Container init not implemented!");
		errno = ENOSYS;
		return NULL;
	}

	return ops->init(io);
}

gp_container *gp_container_open(const char *path)
{
	gp_io *io = gp_io_file(path, GP_IO_RDONLY);
	gp_container *ret;

	ret = gp_container_init(io);
	if (!ret) {
		gp_io_close(io);
		return NULL;
	}

	return ret;
}
