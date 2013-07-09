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

#include "core/GP_Debug.h"

#include "loaders/GP_ByteUtils.h"
#include "loaders/GP_JPG.h"

#include "loaders/GP_ZIP.h"

#ifdef HAVE_ZLIB

#define GEN_FLAG_ENCRYPTED 0x01

struct zip_local_header {
	uint16_t ver;
	uint16_t bit_flags;
	uint16_t comp_type;

	uint32_t crc;
	uint32_t comp_size;
	uint32_t uncomp_size;

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

const char *compress_method_names[] = {
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

static void read_stored(FILE *f, struct zip_local_header *header)
{
	/* ignored for now */
	seek_bytes(f, header->uncomp_size);
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
		errno = EINVAL;
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

static int zip_next_file(FILE *f, FILE **res)
{
	uint16_t fname_len, extf_len;
	struct zip_local_header header = {.file_name = NULL};
	int ret, err = 0;

	ret = GP_FRead(f, "0x50 0x4b 0x03 0x04 L2 L2 L2 I4 L4 L4 L4 L2 L2",
	               &header.ver, &header.bit_flags, &header.comp_type,
		       &header.crc, &header.comp_size, &header.uncomp_size,
	               &fname_len, &extf_len);

	if (ret != 13) {
		GP_DEBUG(1, "Failed to read header");
		return EIO;
	}

	GP_DEBUG(1, "Have ZIP local header version %u.%u compression %s",
	         header.ver/10, header.ver%10,
	         compress_method_name(header.comp_type));

	if (header.bit_flags & GEN_FLAG_ENCRYPTED) {
		GP_DEBUG(1, "Can't handle encrypted files");
		return ENOSYS;
	}

	/*
	 * If input was taken from stdin the fname_len is either set to zero or
	 * to one and filename is set to '-'.
	 */
	if (fname_len) {
		header.file_name = malloc(fname_len + 1);

		if (!header.file_name)
			return ENOMEM;

		header.file_name[fname_len] = '\0';

		if (fread(header.file_name, fname_len, 1, f) != 1) {
			GP_DEBUG(1, "Failed to read filename");
			err = EIO;
			goto err0;
		}

		GP_DEBUG(1, "Filename '%s' compressed size=%"PRIu32
		            " uncompressed size=%"PRIu32,
		            header.file_name, header.comp_size,
		            header.uncomp_size);
	}

	seek_bytes(f, extf_len);

	switch (header.comp_type) {
	case COMPRESS_STORED:
		read_stored(f, &header);
	break;
	case COMPRESS_DEFLATE:
		read_deflate(f, &header, res);
	break;
	default:
		GP_DEBUG(1, "Unimplemented compression %s",
		         compress_method_name(header.comp_type));
		return ENOSYS;
	}

	return 0;
err0:
	free(header.file_name);
	return err;
}

struct zip_priv {
	FILE *f;
};

static GP_Context *zip_load_next(GP_Container *self,
                                 GP_ProgressCallback *callback)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);
	FILE *res = NULL;
	GP_Context *ret;
	int err;

	GP_DEBUG(1, "Trying to load next image from ZIP container");

	do
		err = zip_next_file(priv->f, &res);
	while (!err && !res);

	if (err) {
		errno = err;
		return NULL;
	}

	ret = GP_ReadJPG(res, callback);
	err = errno;
	fclose(res);
	errno = err;

	return ret;
}

static void zip_close(GP_Container *self)
{
	struct zip_priv *priv = GP_CONTAINER_PRIV(self);

	GP_DEBUG(1, "Closing ZIP container");

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
	.Close = zip_close,
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

	priv = GP_CONTAINER_PRIV(ret);

	priv->f = f;

	ret->img_count = -1;
	ret->cur_img = 0;
	ret->ops = &zip_ops;

	return ret;
err0:
	fclose(f);
	errno = err;
	return NULL;
}

#else

GP_Container *GP_OpenZip(const char *path)
{
	GP_FATAL("zlib support not compiled in");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_ZLIB */

