/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "../../config.h"

#ifdef HAVE_ZLIB

#include <zlib.h>

#endif /* HAVE_ZLIB */

#include <core/GP_Common.h>
#include <core/GP_Debug.h>

#include <loaders/GP_ByteUtils.h>
#include <loaders/GP_Loader.h>
#include <loaders/GP_IOZlib.h>
#include "loaders/GP_ZIP.h"

#ifdef HAVE_ZLIB

#define ZIP_CHUNKS_IN_TABLE 128

/*
 * Table used for seeks, populated on the go
 */
struct zip_chunks_table {
	long offsets[ZIP_CHUNKS_IN_TABLE];
	struct zip_chunks_table *next;
	struct zip_chunks_table *prev;
};

struct zip_priv {
	GP_IO *io;

	/* Current position in zip continer counted in images we found */
	unsigned int cur_pos;

	/* Current table */
	unsigned int cur_table_pos;
	struct zip_chunks_table *cur_table;

	/* Last table and index into it, this is used for addition */
	unsigned int tables_used;
	unsigned int table_used;
	struct zip_chunks_table *last_table;

	struct zip_chunks_table table;
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

static int seek_bytes(GP_IO *io, uint32_t bytes)
{
	if (bytes == 0)
		return 0;

	GP_DEBUG(4, "Moving forward by %"PRIu32" bytes", bytes);

	if (GP_IOSeek(io, bytes, GP_IO_SEEK_CUR) == (off_t)-1) {
		int err = errno;
		GP_DEBUG(1, "Failed to seek: %s", strerror(errno));
		return err;
	}

	return 0;
}

static int zip_load_header(GP_IO *io, struct zip_local_header *header)
{
	int ret;
	uint8_t byte;

	uint16_t zip_header[] = {
		'P',
		'K',
		GP_IO_BYTE,
		GP_IO_END
	};

	if (GP_IOReadF(io, zip_header, &byte) != 3) {
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

	ret = GP_IOReadF(io, zip_local_header,
	               &header->ver, &header->flags, &header->comp_type,
		       &header->crc, &header->comp_size, &header->uncomp_size,
	               &header->fname_len, &header->extf_len);

	if (ret != 10) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	return 0;
}

static int zip_read_data_desc(GP_IO *io, struct zip_local_header *header)
{
	uint16_t data_desc_header[] = {
		'P', 'K', 0x07, 0x08, /* Data desc header */
		GP_IO_L4, /* CRC */
		GP_IO_L4, /* Compressed size */
		GP_IO_L4, /* Uncompressed size */
		GP_IO_END
	};

	if (GP_IOReadF(io, data_desc_header, &header->crc,
	               &header->comp_size, &header->uncomp_size) != 7) {
		GP_DEBUG(1, "Failed to read Data Description Header");
		return 1;
	}

	return 0;
}

static GP_Context *zip_next_file(struct zip_priv *priv,
                                 GP_ProgressCallback *callback)
{
	struct zip_local_header header = {.file_name = NULL};
	int err = 0;
	GP_Context *ret = NULL;
	GP_IO *io;

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
		if (GP_IORead(priv->io, header.file_name, header.fname_len) != header.fname_len) {
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

		GP_IOMark(priv->io, GP_IO_MARK);

		ret = GP_ReadImage(priv->io, callback);
		if (errno == ECANCELED)
			err = errno;

		GP_IOSeek(priv->io, priv->io->mark + header.comp_size, GP_IO_SEEK_SET);

		goto out;
	break;
	case COMPRESS_DEFLATE:
	/*	if ((err = read_deflate(priv->io, &header, &io))) {
			err = errno;
			goto out;
		}
		GP_DEBUG(1, "Reading image");
		ret = GP_ReadImage(io, callback);
		if (errno == ECANCELED)
			err = errno;

		GP_IOClose(io);
		goto out;
	*/
		io = GP_IOZlib(priv->io, header.comp_size);
		if (!io)
			goto out;

		GP_DEBUG(1, "Reading image");
		ret = GP_ReadImage(io, callback);
		if (errno == ECANCELED)
			err = errno;

		GP_IOClose(io);

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
	return ret;
}

static unsigned int last_offset_idx(struct zip_priv *priv)
{
	return priv->table_used + priv->tables_used * ZIP_CHUNKS_IN_TABLE;
}

static long last_recorded_offset(struct zip_priv *priv)
{
	const unsigned int last_idx = ZIP_CHUNKS_IN_TABLE - 1;

	if (priv->table_used == 0) {
		if (priv->last_table->prev)
			return priv->last_table->prev->offsets[last_idx];

		return -1;
	}

	return priv->last_table->offsets[priv->table_used - 1];
}

static void record_offset(struct zip_priv *priv, long offset)
{
	if (offset <= last_recorded_offset(priv))
		return;

	GP_DEBUG(2, "Recording offset to %i image (%li)",
	         last_offset_idx(priv), offset);

	if (priv->table_used >= ZIP_CHUNKS_IN_TABLE) {
		struct zip_chunks_table *new_table;

		GP_DEBUG(1, "Allocating chunks table (table nr. %u) (size %i)",
		         priv->tables_used+1, ZIP_CHUNKS_IN_TABLE);

		new_table = malloc(sizeof(struct zip_chunks_table));

		if (!new_table) {
			GP_WARN("Malloc failed :(");
			return;
		}

		priv->tables_used++;
		priv->table_used = 0;
		new_table->prev = priv->last_table;
		new_table->next = NULL;
		priv->last_table->next = new_table;
		priv->last_table = new_table;
	}

	priv->last_table->offsets[priv->table_used++] = offset;
/*
	printf("OFFSET table\n");
	unsigned int i;
	for (i = 0; i < priv->table_used; i++)
		printf("** %u -> %li\n", i, priv->last_table->offsets[i]);
 */
}

static GP_Context *zip_load_next(GP_Container *self,
                                 GP_ProgressCallback *callback)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	GP_Context *ret;
	long offset;

	GP_DEBUG(1, "Trying to load next image from ZIP container");

	do {
		offset = GP_IOTell(priv->io);
		ret = zip_next_file(priv, callback);
	} while (ret == NULL && errno == 0);

	if (!ret)
		return NULL;

	if (ret)
		record_offset(priv, offset);

	record_offset(priv, GP_IOTell(priv->io));

	priv->cur_pos++;
	//self->cur_img++;
	self->cur_img = priv->cur_pos;

	return ret;
}

/* Seek to the current position */
static void seek_cur_pos(struct zip_priv *priv)
{
	unsigned int cur_table = priv->cur_pos / ZIP_CHUNKS_IN_TABLE;
	unsigned int cur_pos;

	if (priv->cur_table_pos != cur_table) {
		unsigned int i;

		GP_DEBUG(3, "cur_pos %u out of cur table %u",
		         priv->cur_pos, priv->cur_table_pos);

		priv->cur_table = &priv->table;

		for (i = 0; i < cur_table; i++) {
			if (priv->cur_table->next)
				priv->cur_table = priv->cur_table->next;
			else
				GP_WARN("The cur_pos points after last table");
		}

		priv->cur_table_pos = cur_table;
	}

	//TODO: Asert that we are not in last table and cur_pos < table_used

	cur_pos = priv->cur_pos % ZIP_CHUNKS_IN_TABLE;

	GP_DEBUG(2, "Setting current position to %u (%li)",
	         priv->cur_pos, priv->cur_table->offsets[cur_pos]);

	GP_IOSeek(priv->io, priv->cur_table->offsets[cur_pos], GP_IO_SEEK_SET);
}

static int load_next_offset(struct zip_priv *priv)
{
	struct zip_local_header header = {.file_name = NULL};
	int ret;
	long offset = GP_IOTell(priv->io);

	if ((ret = zip_load_header(priv->io, &header)))
		return ret;

	//TODO: Match image extension and signature
	record_offset(priv, offset);

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
	unsigned int max = last_offset_idx(priv);
	int err;

	GP_DEBUG(2, "where %u max %u", where, max);

	/* Move to the max and beyond */
	if (where >= max) {
		if (max == 0) {
			if ((err = load_next_offset(priv)))
				return err;
			priv->cur_pos = 0;
		} else {
			priv->cur_pos = max - 1;
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

static int zip_seek(GP_Container *self, int offset,
                    enum GP_ContainerWhence whence)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	unsigned int where;
	int ret;

	GP_DEBUG(1, "Seek offset=%i whence=%i", offset, whence);

	switch (whence) {
	case GP_CONT_CUR:
		if (offset < 0 && priv->cur_pos < (unsigned int)-offset) {
			GP_WARN("Current position %u offset %i",
			        priv->cur_pos, offset);
			where = 0;
		} else {
			where = priv->cur_pos + offset;
		}
	break;
	case GP_CONT_FIRST:
		where = offset;
	break;
	default:
		return ENOSYS;
	}

	ret = set_cur_pos(priv, where);

	self->cur_img = priv->cur_pos;

	return ret;
}

static GP_Context *zip_load(GP_Container *self,
                            GP_ProgressCallback *callback)
{
	GP_Context *img;

	img = zip_load_next(self, callback);

	if (!img)
		return NULL;

	zip_seek(self, -1, GP_CONT_CUR);

	return img;
}

static void zip_close(GP_Container *self)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	struct zip_chunks_table *i, *j;

	GP_DEBUG(1, "Closing ZIP container");

	/* Free allocated offset tables */
	for (i = priv->table.next; i != NULL; j = i, i = i->next, free(j));

	GP_IOClose(priv->io);
	free(self);
}

static GP_IO *open_zip(const char *path)
{
	GP_IO *io;
	int err = 0;

	io = GP_IOFile(path, GP_IO_RDONLY);

	if (!io) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s': %s", path, strerror(errno));
		goto err0;
	}

	/* Check zip local file header and seek back */
	if (GP_IOMark(io, GP_IO_MARK)) {
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

	if (GP_IOReadF(io, zip_header) != 4) {
		GP_DEBUG(1, "Invalid zip header");
		err = EINVAL;
		goto err1;
	}

	if (GP_IOMark(io, GP_IO_REWIND)) {
		err = errno;
		goto err1;
	}

	return io;
err1:
	GP_IOClose(io);
err0:
	errno = err;
	return NULL;
}

static const struct GP_ContainerOps zip_ops = {
	.LoadNext = zip_load_next,
	.Load = zip_load,
	.Close = zip_close,
	.Seek = zip_seek,
	.type = "ZIP",
};

GP_Container *GP_OpenZip(const char *path)
{
	struct zip_priv *priv;
	GP_Container *ret;
	GP_IO *io;
	int err;

	io = open_zip(path);

	if (!io)
		return NULL;

	ret = malloc(sizeof(GP_Container) + sizeof(struct zip_priv));

	if (!ret) {
		err = ENOMEM;
		goto err0;
	}

	GP_DEBUG(1, "ZIP Container initialized");

	ret->img_count = -1;
	ret->cur_img = 0;
	ret->ops = &zip_ops;

	priv = GP_CONTAINER_PRIV(ret);

	priv->io = io;

	priv->table.next = NULL;
	priv->table.prev = NULL;

	/* Cache for current table for seeks */
	priv->cur_table = &priv->table;
	priv->cur_table_pos = 0;

	/* Current position */
	priv->cur_pos = 0;

	/* Last table, used for insertion */
	priv->tables_used = 0;
	priv->table_used = 0;
	priv->last_table = &priv->table;

	return ret;
err0:
	GP_IOClose(io);
	errno = err;
	return NULL;
}

#else

GP_Container *GP_OpenZip(const char GP_UNUSED(*path))
{
	GP_FATAL("zlib support not compiled in");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_ZLIB */

int GP_MatchZip(const char *buf)
{
	return !memcmp("PK\03\04", buf, 4);
}
