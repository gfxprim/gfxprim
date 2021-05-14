// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#ifndef GFXPRIM_NO_CONFIG
#include "../../config.h"
#endif

#ifdef HAVE_ZLIB

#include <zlib.h>

#endif /* HAVE_ZLIB */

#include <utils/gp_vec.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>

#include <loaders/gp_loader.h>
#include <loaders/gp_io_zlib.h>
#include <loaders/gp_zip.h>

#ifdef HAVE_ZLIB

struct zip_priv {
	gp_io *io;

	/* Current position in zip continer counted in images we found */
	unsigned int cur_pos;

	/* Offsets to zip local headers */
	long *offsets;
};

struct zip_local_header {
	uint16_t ver;
	uint16_t flags;
	uint16_t comp_type;

	uint32_t crc;
	uint32_t comp_size;
	uint32_t uncomp_size;

	uint16_t fname_len;
	uint16_t extf_len;

	char *file_name;
};

enum compress_method {
	COMPRESS_STORED         = 0x00, /* no compression at all */
	COMPRESS_SHRUNK         = 0x01,
	COMPRESS_REDUCE1        = 0x02,
	COMPRESS_REDUCE2        = 0x03,
	COMPRESS_REDUCE3        = 0x04,
	COMPRESS_REDUCE4        = 0x05,
	COMPRESS_IMPLODE        = 0x06,
	COMPRESS_RESERVED1      = 0x07,
	COMPRESS_DEFLATE        = 0x08,
	COMPRESS_DEFLATE64      = 0x09,
	COMPRESS_PKWARE_IMPLODE = 0x0a,
	COMPRESS_RESERVED2      = 0x0b,
	COMPRESS_BZIP2          = 0x0c,
};

static const char *compress_method_names[] = {
	"Stored (no compression)",
	"Shrunk",
	"Reduced with factor 1",
	"Reduced with factor 2",
	"Reduced with factor 3",
	"Reduced with factor 4",
	"Imploded",
	"Reserved 1",
	"Deflate",
	"Deflate64",
	"PKWARE Implode",
	"Reserved 2",
	"BZIP2",
};

static const char *compress_method_name(enum compress_method comp)
{
	if (comp > COMPRESS_BZIP2)
		return "Unknown";

	return compress_method_names[comp];
}

enum zip_flags {
	/* File is encrypted */
	FLAG_ENCRYPTED = 0x0001,
	/* Size and CRC are in data descriptor after the compressed data */
	FLAG_DATA_DESC_HEADER = 0x0008,
	/* Filename and comment fileds are in UTF-8 */
	FLAG_UTF8 = 0x0800,
};

static void print_flags(struct zip_local_header *header)
{
	if (header->flags & FLAG_ENCRYPTED)
		GP_DEBUG(2, "File is encrypted");

	if (header->flags & FLAG_DATA_DESC_HEADER)
		GP_DEBUG(2, "File size and CRC are after compressed data");

	if (header->flags & FLAG_UTF8)
		GP_DEBUG(2, "Filename and comment are encoded in UTF-8");
}

static int seek_bytes(gp_io *io, uint32_t bytes)
{
	if (bytes == 0)
		return 0;

	GP_DEBUG(4, "Moving forward by %"PRIu32" bytes", bytes);

	if (gp_io_seek(io, bytes, GP_SEEK_CUR) == (off_t)-1) {
		int err = errno;
		GP_DEBUG(1, "Failed to seek: %s", strerror(errno));
		return err;
	}

	return 0;
}

static int zip_load_header(gp_io *io, struct zip_local_header *header)
{
	int ret;
	uint8_t byte;

	uint16_t zip_header[] = {
		'P',
		'K',
		GP_IO_BYTE,
		GP_IO_END
	};

	if (gp_io_readf(io, zip_header, &byte) != 3) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	switch (byte) {
	/* Central directory -> end of archive */
	case 0x01:
		GP_DEBUG(1, "Reached end of the archive");
		return EINVAL;
	break;
	/* File header */
	case 0x03:
	break;
	default:
		GP_DEBUG(1, "Unexpected header PK%x", byte);
		return EIO;
	}

	uint16_t zip_local_header[] = {
		0x04,
		GP_IO_L2, /* version */
		GP_IO_L2, /* bit flags */
		GP_IO_L2, /* compression type */
		GP_IO_IGN | 4,
		GP_IO_L4, /* CRC */
		GP_IO_L4, /* compressed size */
		GP_IO_L4, /* uncompressed size */
		GP_IO_L2, /* filename length */
		GP_IO_L2, /* extra fields lenght */
		GP_IO_END
	};

	ret = gp_io_readf(io, zip_local_header,
	               &header->ver, &header->flags, &header->comp_type,
		       &header->crc, &header->comp_size, &header->uncomp_size,
	               &header->fname_len, &header->extf_len);

	if (ret != 10) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	return 0;
}

static int zip_read_data_desc(gp_io *io, struct zip_local_header *header)
{
	uint16_t data_desc_header[] = {
		'P', 'K', 0x07, 0x08, /* Data desc header */
		GP_IO_L4, /* CRC */
		GP_IO_L4, /* Compressed size */
		GP_IO_L4, /* Uncompressed size */
		GP_IO_END
	};

	if (gp_io_readf(io, data_desc_header, &header->crc,
	               &header->comp_size, &header->uncomp_size) != 7) {
		GP_DEBUG(1, "Failed to read Data Description Header");
		return 1;
	}

	GP_DEBUG(1, "Read Data Description Header");

	return 0;
}

static int zip_next_file(struct zip_priv *priv, gp_pixmap **img,
                         gp_storage *storage,
                         gp_progress_cb *callback)
{
	struct zip_local_header header = {.file_name = NULL};
	int err = 0, res;
	gp_pixmap *ret = NULL;
	gp_io *io;

	if ((err = zip_load_header(priv->io, &header)))
		goto out;

	GP_DEBUG(1, "Have ZIP local header version %u.%u compression %s",
	         header.ver/10, header.ver%10,
	         compress_method_name(header.comp_type));

	print_flags(&header);

	if (header.flags & FLAG_ENCRYPTED) {
		GP_DEBUG(1, "Can't handle encrypted files");
		err = ENOSYS;
		goto out;
	}

	/*
	 * If input was taken from stdin the fname_len is either set to zero or
	 * to one and filename is set to '-'.
	 */
	if (header.fname_len) {
		header.file_name = malloc(header.fname_len + 1);

		if (!header.file_name) {
			err = ENOMEM;
			goto out;
		}

		header.file_name[header.fname_len] = '\0';
		//FILL
		if (gp_io_read(priv->io, header.file_name, header.fname_len) != header.fname_len) {
			GP_DEBUG(1, "Failed to read filename");
			err = EIO;
			goto out;
		}

		GP_DEBUG(1, "Filename '%s' compressed size=%"PRIu32
		            " uncompressed size=%"PRIu32,
		            header.file_name, header.comp_size,
		            header.uncomp_size);
	}

	seek_bytes(priv->io, header.extf_len);

	switch (header.comp_type) {
	case COMPRESS_STORED:
		/* skip directories */
		if (header.uncomp_size == 0) {
			GP_DEBUG(2, "Skipping directory");
			goto out;
		}

		gp_io_mark(priv->io, GP_IO_MARK);

		res = gp_read_image_ex(priv->io, &ret, storage, callback);
		if (res && errno == ECANCELED)
			err = errno;

		gp_io_seek(priv->io, priv->io->mark + header.comp_size, GP_SEEK_SET);

		goto out;
	break;
	case COMPRESS_DEFLATE:
		io = gp_io_zlib(priv->io, header.comp_size);
		if (!io) {
			err = errno;
			goto out;
		}

		GP_DEBUG(1, "Reading image");
		res = gp_read_image_ex(io, &ret, storage, callback);
		if (res && errno == ECANCELED)
			err = errno;

		/*
		 * We need to finish the Zlib IO for any of:
		 *
		 * - File is not image -> need to get to the end of the record
		 * - All image data were not consumed by loader (may happen)
		 */
		if (gp_io_seek(io, 0, GP_SEEK_END) == (off_t)-1)
			GP_DEBUG(1, "Failed to seek Zlib IO");

		gp_io_close(io);

		if (header.flags & FLAG_DATA_DESC_HEADER) {
			if (zip_read_data_desc(priv->io, &header))
				goto out;
		}

		goto out;
	break;
	default:
		GP_DEBUG(1, "Unimplemented compression %s",
		         compress_method_name(header.comp_type));
		err = ENOSYS;
		goto out;
	}

out:
	free(header.file_name);
	errno = err;
	*img = ret;
	return err;
}

static void record_offset(struct zip_priv *priv, size_t pos, long offset)
{
	size_t last_off = gp_vec_len(priv->offsets);
	long *offsets;

	if (pos < last_off)
		return;

	if (pos != last_off) {
		GP_ABORT("Invalid offset");
		return;
	}

	GP_DEBUG(2, "Recording offset to %zu image (%li)", pos, offset);

	offsets = gp_vec_append(priv->offsets, 1);
	if (!offsets) {
		GP_WARN("Failed to grow offsets vector");
		return;
	}

	priv->offsets = offsets;
	offsets[pos] = offset;
}

static int zip_load_next(gp_container *self, gp_pixmap **img,
                         gp_storage *storage,
                         gp_progress_cb *callback)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	int err;

	GP_DEBUG(1, "Trying to load next image from ZIP container");

	*img = NULL;

	do {
		err = zip_next_file(priv, img, storage, callback);
	} while (!*img && errno == 0);

	if (err)
		return 1;

	priv->cur_pos++;
	self->cur_img = priv->cur_pos;

	record_offset(priv, priv->cur_pos, gp_io_tell(priv->io));

	return 0;
}

static gp_pixmap *load_next(gp_container *self, gp_progress_cb *callback)
{
	gp_pixmap *img = NULL;

	zip_load_next(self, &img, NULL, callback);

	return img;
}

/* Seek to the current position */
static void seek_cur_pos(struct zip_priv *priv)
{
	size_t last_off = gp_vec_len(priv->offsets);

	if (priv->cur_pos >= last_off) {
		GP_WARN("Attempt to seek out of the offsets table");
		return;
	}

	GP_DEBUG(2, "Setting current position to %u (%li)",
	         priv->cur_pos, priv->offsets[priv->cur_pos]);

	gp_io_seek(priv->io, priv->offsets[priv->cur_pos], GP_SEEK_SET);
}

static int load_next_offset(struct zip_priv *priv)
{
	struct zip_local_header header = {.file_name = NULL};
	int ret;
	long offset = gp_io_tell(priv->io);

	if ((ret = zip_load_header(priv->io, &header)))
		return ret;

	//TODO: Match image extension and signature
	record_offset(priv, priv->cur_pos + 1, offset);

	if (!header.fname_len || !header.extf_len)
		GP_WARN("Wrong header size!");

	/* Seek to the next local header */
	seek_bytes(priv->io, (uint32_t)header.fname_len +
	                    (uint32_t)header.extf_len);
	seek_bytes(priv->io, header.comp_size);

	return 0;
}

/*
 * Sets current position.
 */
static int set_cur_pos(struct zip_priv *priv, unsigned int where)
{
	size_t last_off = gp_vec_len(priv->offsets)-1;
	int err;

	GP_DEBUG(2, "where %u max %zu", where, last_off);

	/* Move to the max and beyond */
	if (where > last_off) {
		if (last_off == 0) {
			if ((err = load_next_offset(priv)))
				return err;
			priv->cur_pos = 0;
		} else {
			priv->cur_pos = last_off - 1;
			seek_cur_pos(priv);
		}

		while (priv->cur_pos < where) {
			if ((err = load_next_offset(priv)))
				return err;
			priv->cur_pos++;
		}

		return 0;
	}

	priv->cur_pos = where;
	seek_cur_pos(priv);

	return 0;
}

static int zip_seek(gp_container *self, ssize_t offset,
                    enum gp_seek_whence whence)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	ssize_t where;
	int ret;

	GP_DEBUG(1, "Seek offset=%zi whence=%i", offset, whence);

	switch (whence) {
	case GP_SEEK_CUR:
		if (offset < 0 && priv->cur_pos < (unsigned int)-offset) {
			GP_WARN("Current position %u offset %zi",
			        priv->cur_pos, offset);
			where = 0;
		} else {
			where = priv->cur_pos + offset;
		}
	break;
	case GP_SEEK_SET:
		where = offset;
	break;
	default:
		return ENOSYS;
	}

	ret = set_cur_pos(priv, where);

	self->cur_img = priv->cur_pos;

	return ret;
}

static int zip_load(gp_container *self, gp_pixmap **img,
                    gp_storage *storage, gp_progress_cb *callback)
{
	if (zip_load_next(self, img, storage, callback))
		return 1;

	zip_seek(self, -1, GP_SEEK_CUR);

	return 0;
}

static void zip_close(gp_container *self)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);

	GP_DEBUG(1, "Closing ZIP container");

	gp_vec_free(priv->offsets);
	gp_io_close(priv->io);
	free(self);
}

static gp_io *open_zip(const char *path)
{
	gp_io *io;
	int err = 0;

	io = gp_io_file(path, GP_IO_RDONLY);

	if (!io) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s': %s", path, strerror(errno));
		goto err0;
	}

	/* Check zip local file header and seek back */
	if (gp_io_mark(io, GP_IO_MARK)) {
		err = errno;
		goto err1;
	}

	static uint16_t zip_header[] = {
		'P',
		'K',
		0x03,
		0x04,
		GP_IO_END
	};

	if (gp_io_readf(io, zip_header) != 4) {
		GP_DEBUG(1, "Invalid zip header");
		err = EINVAL;
		goto err1;
	}

	if (gp_io_mark(io, GP_IO_REWIND)) {
		err = errno;
		goto err1;
	}

	return io;
err1:
	gp_io_close(io);
err0:
	errno = err;
	return NULL;
}

gp_container *gp_init_zip(gp_io *io)
{
	struct zip_priv *priv;
	gp_container *ret;
	int err;
	long *offsets;

	ret = malloc(sizeof(gp_container) + sizeof(struct zip_priv));
	offsets = gp_vec_new(1, sizeof(long));

	if (!ret || !offsets) {
		err = ENOMEM;
		free(ret);
		gp_vec_free(offsets);
		goto err0;
	}

	GP_DEBUG(1, "ZIP Container initialized");

	ret->img_count = -1;
	ret->cur_img = 0;
	ret->ops = &gp_zip_ops;

	priv = GP_CONTAINER_PRIV(ret);

	priv->io = io;
	priv->cur_pos = 0;
	priv->offsets = offsets;

	return ret;
err0:
	gp_io_close(io);
	errno = err;
	return NULL;
}

gp_container *gp_open_zip(const char *path)
{
	gp_container *ret;
	gp_io *io;

	io = open_zip(path);
	if (!io)
		return NULL;

	ret = gp_init_zip(io);
	if (!ret) {
		int err = errno;
		gp_io_close(io);
		errno = err;
		return NULL;
	}

	return ret;
}

#else

gp_container *gp_open_zip(const char GP_UNUSED(*path))
{
	GP_FATAL("zlib support not compiled in");
	errno = ENOSYS;
	return NULL;
}

gp_container *gp_init_zip(gp_io GP_UNUSED(*io))
{
	GP_FATAL("zlib support not compiled in");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_ZLIB */

int gp_match_zip(const void *buf)
{
	return !memcmp("PK\03\04", buf, 4);
}

const gp_container_ops gp_zip_ops = {
#ifdef HAVE_ZLIB
	.load_next = load_next,
	.load_ex = zip_load,
	.close = zip_close,
	.seek = zip_seek,
#endif /* HAVE_ZLIB */
	.init = gp_init_zip,
	.match = gp_match_zip,
	.fmt_name = "ZIP",
	.extensions = {"zip", "cbz", NULL}
};
