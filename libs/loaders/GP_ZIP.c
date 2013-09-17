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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
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

#include "core/GP_Common.h"
#include "core/GP_Debug.h"

#include "loaders/GP_ByteUtils.h"
#include "loaders/GP_JPG.h"
#include "loaders/GP_PNG.h"

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
	FILE *f;

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

#define GEN_FLAG_ENCRYPTED 0x01

struct zip_local_header {
	uint16_t ver;
	uint16_t bit_flags;
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

static int seek_bytes(FILE *f, uint32_t bytes)
{
	if (bytes == 0)
		return 0;

	GP_DEBUG(4, "Moving forward by %"PRIu32"bytes", bytes);

	if (fseek(f, bytes, SEEK_CUR)) {
		int err = errno;
		GP_DEBUG(1, "Failed to seek: %s", strerror(errno));
		return err;
	}

	return 0;
}

#define CHUNK 512

struct deflate_inbuf {
	struct zip_local_header *zip_header;
	uint32_t to_read;
	unsigned char buf[CHUNK];
	FILE *f;
};

struct deflate_outbuf {
	uint32_t crc;
	uint32_t size;
	FILE *f;
};

static unsigned deflate_in(void *in_desc, unsigned char **buf)
{
	struct deflate_inbuf *in = in_desc;
	uint32_t chunk = in->to_read >= CHUNK ? CHUNK : in->to_read;

	if (fread(in->buf, chunk, 1, in->f) != 1)
		return 0;

	*buf = in->buf;
	in->to_read -= chunk;

	return chunk;
}

static int deflate_out(void *out_desc, unsigned char *buf, unsigned len)
{
	struct deflate_outbuf *out = out_desc;

	out->crc = crc32(out->crc, buf, len);
	out->size += len;

	if (fwrite(buf, len, 1, out->f) != 1) {
		GP_DEBUG(1, "Failed to write temp file");
		return 1;
	}

	return 0;
}

static int read_deflate(FILE *f, struct zip_local_header *header, FILE **res_f)
{
	uint8_t *window;
	int err = 0, ret;
	FILE *tf;

	tf = tmpfile();

	if (!tf) {
		err = errno;
		GP_DEBUG(1, "Failed to create temp file");
		return err;
	}

	window = malloc(32 * 1024);

	if (!window) {
		err = ENOMEM;
		goto err0;
	}

	z_stream strm = {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
		.next_in = Z_NULL,
	};

	if (inflateBackInit(&strm, 15, window) != Z_OK) {
		GP_DEBUG(1, "Failed to initialize inflate stream");
		//TODO: Better errno?
		err = EIO;
		goto err1;
	}

	struct deflate_outbuf outbuf = {
		.crc = crc32(0, NULL, 0),
		.size = 0,
		.f = tf,
	};

	struct deflate_inbuf inbuf = {
		.zip_header = header,
		.f = f,
		.to_read = header->comp_size,
	};

	ret = inflateBack(&strm, deflate_in, &inbuf, deflate_out, &outbuf);

	if (ret != Z_STREAM_END) {
		GP_DEBUG(1, "Failed to inflate stream %i", ret);
		err = EINVAL;
		goto err2;
	}

	if (outbuf.crc != header->crc) {
		GP_DEBUG(1, "CRC does not match");
		err = EIO;
		goto err2;
	}

	if (outbuf.size != header->uncomp_size) {
		GP_DEBUG(1, "Decompressed size does not match");
		err = EIO;
		goto err2;
	}

	rewind(tf);
	*res_f = tf;
	return 0;
err2:
	inflateBackEnd(&strm);
err1:
	free(window);
err0:
	fclose(tf);
	return err;
}

static int zip_load_header(FILE *f, struct zip_local_header *header)
{
	int ret;

	//TODO: check for central directory signature -> end of data

	ret = GP_FRead(f, "0x50 0x4b 0x03 0x04 L2 L2 L2 I4 L4 L4 L4 L2 L2",
	               &header->ver, &header->bit_flags, &header->comp_type,
		       &header->crc, &header->comp_size, &header->uncomp_size,
	               &header->fname_len, &header->extf_len);

	if (ret != 13) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	return 0;
}

static GP_Context *zip_next_file(FILE *f, GP_ProgressCallback *callback)
{
	struct zip_local_header header = {.file_name = NULL};
	int err = 0;
	GP_Context *ret = NULL;
	FILE *fres;

	if ((err = zip_load_header(f, &header)))
		goto out;

	GP_DEBUG(1, "Have ZIP local header version %u.%u compression %s",
	         header.ver/10, header.ver%10,
	         compress_method_name(header.comp_type));

	if (header.bit_flags & GEN_FLAG_ENCRYPTED) {
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

		if (fread(header.file_name, header.fname_len, 1, f) != 1) {
			GP_DEBUG(1, "Failed to read filename");
			err = EIO;
			goto out;
		}

		GP_DEBUG(1, "Filename '%s' compressed size=%"PRIu32
		            " uncompressed size=%"PRIu32,
		            header.file_name, header.comp_size,
		            header.uncomp_size);
	}

	seek_bytes(f, header.extf_len);

	switch (header.comp_type) {
	case COMPRESS_STORED:
		/* skip directories */
		if (header.uncomp_size == 0)
			goto out;

		off_t cur_off = ftell(f);

		ret = GP_ReadJPG(f, callback);

		if (!ret) {
			fseek(f, cur_off, SEEK_SET);
			ret = GP_ReadPNG(f, callback);
		}

		goto out;
	break;
	case COMPRESS_DEFLATE:
		if (read_deflate(f, &header, &fres))
			goto out;


		ret = GP_ReadJPG(fres, callback);

		if (!ret) {
			rewind(fres);
			ret = GP_ReadPNG(fres, callback);
		}

		if (!ret)
			err = errno;

		fclose(fres);
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
		offset = ftell(priv->f);
		ret = zip_next_file(priv->f, callback);
	} while (ret == NULL && errno == 0);

	if (!ret)
		return NULL;

	if (ret)
		record_offset(priv, offset);

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

	fseek(priv->f, priv->cur_table->offsets[cur_pos], SEEK_SET);
}

static int load_next_offset(struct zip_priv *priv)
{
	struct zip_local_header header = {.file_name = NULL};
	int ret;
	long offset = ftell(priv->f);

	//TODO: End of file!
	if ((ret = zip_load_header(priv->f, &header)))
		return ret;

	//TODO: Match image extension and signature
	record_offset(priv, offset);

	/* Seek to the next local header */
	seek_bytes(priv->f, (uint32_t)header.fname_len +
	                    (uint32_t)header.extf_len);
	seek_bytes(priv->f, header.comp_size);

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

	fclose(priv->f);
	free(self);
}

static int open_zip(const char *path, FILE **file)
{
	FILE *f;
	int err = 0;

	f = fopen(path, "rb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s': %s", path, strerror(errno));
		if (!err)
			err = EIO;
		return err;
	}

	/* Check zip local file header and seek back */
	if (GP_FRead(f, "0x50 0x4b 0x03 0x04") != 4) {
		GP_DEBUG(1, "Invalid zip header");
		err = EINVAL;
		goto err0;
	}

	rewind(f);

	*file = f;
	return 0;
err0:
	fclose(f);
	return err;
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
	FILE *f;
	int err;

	if ((err = open_zip(path, &f))) {
		errno = err;
		return NULL;
	}

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

	priv->f = f;

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
	fclose(f);
	errno = err;
	return NULL;
}

#else

GP_Container *GP_OpenZip(const char *path)
{
	(void) path;
	GP_FATAL("zlib support not compiled in");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_ZLIB */

int GP_MatchZip(const char *buf)
{
	return !memcmp("PK\03\04", buf, 4);
}
