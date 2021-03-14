// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>

#include "../../config.h"

#ifdef HAVE_LIBARCHIVE
# include <archive.h>
# include <archive_entry.h>
#endif /* HAVE_LIBARCHIVE */

#include <utils/gp_vec.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>

#include <loaders/gp_loader.h>
#include <loaders/gp_rar.h>

#ifdef HAVE_LIBARCHIVE

struct rar_priv {
	gp_io *io;
	struct archive *a;
	size_t pos;
	char buf[1024];
};

static ssize_t a_read(struct archive GP_UNUSED(*a),
                      void *ptr, const void **buf)
{
	struct rar_priv *priv = ptr;

        *buf = priv->buf;

	return gp_io_read(priv->io, priv->buf, sizeof(priv->buf));
}

static off_t a_seek(struct archive GP_UNUSED(*a),
                    void *ptr, off_t offset, int whence)
{
	struct rar_priv *priv = ptr;

	return gp_io_seek(priv->io, offset, whence);
}

static struct archive *new_archive(struct rar_priv *priv)
{
	struct archive *a;

	a = archive_read_new();
	if (!a) {
		GP_DEBUG(1, "archive_read_new() failed");
		return NULL;
	}

	archive_read_support_format_rar(a);
	archive_read_support_format_rar5(a);
	archive_read_set_read_callback(a, a_read);
	archive_read_set_seek_callback(a, a_seek);
	archive_read_set_callback_data(a, priv);

	archive_read_open1(a);

	return a;
}

gp_container *gp_init_rar(gp_io *io)
{
	struct rar_priv *priv;
	gp_container *ret;

	ret = malloc(sizeof(gp_container) + sizeof(struct rar_priv));
	if (!ret) {
		errno = ENOMEM;
		return NULL;
	}

	priv = GP_CONTAINER_PRIV(ret);

	priv->pos = 0;
	priv->io = io;
	priv->a = new_archive(priv);

	if (!priv->a) {
		free(ret);
		errno = ENOMEM;
		return NULL;
	}

	ret->ops = &gp_rar_ops;
	ret->cur_img = 0;

	return ret;
}

struct entry_io {
	gp_io io;
	struct archive *a;
};

static ssize_t entry_read(gp_io *self, void *buf, size_t size)
{
	struct entry_io *eio = GP_CONTAINER_OF(self, struct entry_io, io);
	ssize_t ret;

	if ((ret = archive_read_data(eio->a, buf, size)) < 0)
		GP_DEBUG(1, "libarchive read: %s", strerror(errno));

	return ret;
}

static off_t entry_seek(gp_io *self, off_t off, enum gp_seek_whence whence)
{
	struct entry_io *eio = GP_CONTAINER_OF(self, struct entry_io, io);
	off_t ret;

	if ((ret = archive_seek_data(eio->a, off, whence)) < 0) {
		switch (ret) {
		case ARCHIVE_FATAL:
			errno = ENOSYS;
		break;
		case ARCHIVE_FAILED:
			errno = EINVAL;
		break;
		}

		GP_DEBUG(1, "libarchive seek: %s", strerror(errno));
		return (off_t)-1;
	}

	return ret;
}

static int load_next(gp_container *self, gp_pixmap **img,
                     gp_storage *storage, gp_progress_cb *callback)
{
	struct rar_priv *priv = GP_CONTAINER_PRIV(self);
	struct archive_entry *a_entry;

	if (archive_read_next_header(priv->a, &a_entry) != ARCHIVE_OK) {
		GP_DEBUG(1, "libarchive: %s", archive_error_string(priv->a));
		errno = EIO;
		return 1;
	}

	struct entry_io eio = {
		.io = {
			.read = entry_read,
			.seek = entry_seek
		},
		.a = priv->a
	};

	GP_DEBUG(1, "Loading image '%s'", archive_entry_pathname(a_entry));

	priv->pos++;
	self->cur_img = priv->pos;

	return gp_read_image_ex(&eio.io, img, storage, callback);
}

static int rar_seek_to(gp_container *self, size_t where)
{
	struct rar_priv *priv = GP_CONTAINER_PRIV(self);
	struct archive_entry *a_entry;
	size_t i, forward;

	if (where < priv->pos) {
		GP_DEBUG(1, "Reopening RAR archive");

		if (gp_io_rewind(priv->io))
			return errno;

		archive_read_free(priv->a);

		priv->a = new_archive(priv);

		forward = where;
	} else {
		forward = where - priv->pos;
	}

	GP_DEBUG(1, "Moving forward %zu", forward);

	for (i = 0; i < forward; i++) {
		if (archive_read_next_header(priv->a, &a_entry) != ARCHIVE_OK) {
			GP_DEBUG(1, "libarchive: %s", archive_error_string(priv->a));
			return EIO;
		}
	}

	priv->pos = where;
	self->cur_img = where;

	return 0;
}

static int rar_seek(gp_container *self, ssize_t offset,
                    enum gp_seek_whence whence)
{
	struct rar_priv *priv = GP_CONTAINER_PRIV(self);

	switch (whence) {
	case GP_SEEK_CUR:
		if (offset < 0 && priv->pos < (size_t)-offset) {
			GP_WARN("Attempt to seek before archive beginning!");
			return EINVAL;
		}

		return rar_seek_to(self, priv->pos + offset);
	break;
	case GP_SEEK_SET:
		if (offset < 0) {
			GP_WARN("Attempt to seek before archive beginning!");
			return EINVAL;
		}

		return rar_seek_to(self, offset);
	break;
	default:
		return ENOSYS;
	}
}

static int rar_load_ex(gp_container *self, gp_pixmap **img,
                       gp_storage *storage, gp_progress_cb *callback)
{
	if (load_next(self, img, storage, callback))
		return 1;

	rar_seek(self, -1, GP_SEEK_CUR);

	return 0;
}

static gp_pixmap *rar_load_next(gp_container *self, gp_progress_cb *callback)
{
        gp_pixmap *img = NULL;

        load_next(self, &img, NULL, callback);

        return img;
}

static void rar_close(gp_container *self)
{
	struct rar_priv *priv = GP_CONTAINER_PRIV(self);

	archive_read_free(priv->a);
	//CLOSE io?
	free(self);
}

#else

gp_container *gp_open_rar(const char GP_UNUSED(*path))
{
	GP_FATAL("libarchive support not compiled in");
	errno = ENOSYS;
	return NULL;
}

gp_container *gp_init_rar(gp_io GP_UNUSED(*io))
{
	GP_FATAL("libarchive support not compiled in");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_LIBARCHIVE */

int gp_match_rar(const void *buf)
{
	return !memcmp("Rar!\x1a\x07\x00", buf, 7) |
	       !memcmp("Rar!\x1a\x07\x01\x00", buf, 8);
}

const gp_container_ops gp_rar_ops = {
#ifdef HAVE_LIBARCHIVE
	.load_next = rar_load_next,
	.load_ex = rar_load_ex,
	.close = rar_close,
	.seek = rar_seek,
#endif /* HAVE_LIBARCHIVE */
	.init = gp_init_rar,
	.match = gp_match_rar,
	.fmt_name = "RAR",
	.extensions = {"rar", "cbr", NULL}
};

