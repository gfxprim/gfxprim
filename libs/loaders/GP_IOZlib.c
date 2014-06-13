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

#include "../../config.h"

#include <errno.h>
#include <stdint.h>
#include <core/GP_Debug.h>
#include <core/GP_Common.h>
#include <loaders/GP_IOZlib.h>

#ifdef HAVE_ZLIB

#include <zlib.h>

#define BUFS 512u

struct priv {
	z_stream strm;

	GP_IO *io;
	off_t io_start;
	int eos;

	uint32_t crc;
	size_t comp_avail;
	size_t comp_size;
	size_t bytes_read;

	char inbuf[BUFS];
};

static int zlib_close(GP_IO *io)
{
	struct priv *priv = GP_IO_PRIV(io);

	GP_DEBUG(1, "Closing IO (%p)", io);

	inflateEnd(&priv->strm);
	free(io);

	return 0;
}

static ssize_t zlib_read(GP_IO *io, void *buf, size_t size)
{
	struct priv *priv = GP_IO_PRIV(io);
	size_t bread;
	int ret;

	GP_DEBUG(3, "Read %p %zu", buf, size);

	if (priv->eos)
		return 0;

	priv->strm.avail_out = size;
	priv->strm.next_out = buf;

	do {
		if (priv->strm.avail_in == 0) {
			size_t to_read = BUFS;

			if (priv->comp_avail)
				to_read = GP_MIN(BUFS, priv->comp_avail);

			ret = GP_IORead(priv->io, priv->inbuf, to_read);

			if (ret <= 0)
				return ret;

			if (priv->comp_avail)
				priv->comp_avail -= ret;

			priv->strm.avail_in = ret;
			priv->strm.next_in = (void*)priv->inbuf;
		}

		//priv->strm.next_out = buf + (size - priv->strm.avail_out);
		ret = inflate(&priv->strm, Z_NO_FLUSH);

		switch (ret) {
		case Z_OK:
		break;
		case Z_STREAM_END:
			GP_DEBUG(1, "End of stream");

			priv->eos = 1;

			/* Attempt to seek back in the parent I/O stream */
			if (priv->strm.avail_in) {
				GP_DEBUG(1, "Seeking back by %zu", (size_t)priv->strm.avail_in);
				GP_IOSeek(priv->io, -(int)priv->strm.avail_in, GP_IO_SEEK_CUR);
			}

			goto out;
		break;
		default:
			GP_DEBUG(1, "inflate() failed %i", ret);
			errno = EIO;
			return -1;
		}
	} while (priv->strm.avail_out == size);

out:
	bread = size - priv->strm.avail_out;
	priv->bytes_read += bread;
	priv->crc = crc32(priv->crc, buf, bread);

	return bread;
}

static int zlib_reset(struct priv *priv)
{
	inflateReset(&priv->strm);

	priv->strm.avail_in = 0;
	priv->strm.next_in = Z_NULL;

	priv->strm.avail_out = 0;
	priv->strm.next_out = Z_NULL;

	priv->bytes_read = 0;
	priv->comp_avail = priv->comp_size;
        priv->crc = crc32(0, NULL, 0);
	priv->eos = 0;

	return 0;
}

static off_t zlib_seek_end(GP_IO *io)
{
	struct priv *priv = GP_IO_PRIV(io);
	char buf[BUFS];
	int ret;

	while ((ret = zlib_read(io, buf, sizeof(buf))) > 0);

	if (ret < 0)
		return (off_t)-1;

	return priv->bytes_read;
}

static off_t zlib_seek(GP_IO *io, off_t offset, enum GP_IOWhence whence)
{
	struct priv *priv = GP_IO_PRIV(io);
	off_t ret;

	GP_DEBUG(3, "Seek %li %u", (long)offset, whence);

	if (whence == GP_IO_SEEK_CUR) {
		if (offset == 0)
			return priv->bytes_read;
		if (offset < 0)
			goto out;
		char b[offset];
		priv->bytes_read += offset;
		GP_IOFill(io, b, offset);
		return priv->bytes_read;
	}

	if (whence == GP_IO_SEEK_SET && offset == 0) {
		ret = GP_IOSeek(priv->io, priv->io_start, GP_IO_SEEK_SET);

		if (ret == (off_t)-1)
			return ret;

		if (zlib_reset(priv))
			return (off_t)-1;

		return 0;
	}

	if (whence == GP_IO_SEEK_END && offset == 0)
		return zlib_seek_end(io);

out:
	errno = ENOSYS;
	return (off_t)-1;
}

int GP_IOZlibReset(GP_IO *io, GP_IO *sub_io, size_t comp_size)
{
	struct priv *priv = GP_IO_PRIV(io);

	GP_DEBUG(1, "Resseting I/O (%p) (parent %p) size %zu",
	         io, sub_io, comp_size);

	priv->io = sub_io;
	priv->comp_size = comp_size;

	return zlib_reset(priv);
}

GP_IO *GP_IOZlib(GP_IO *io, size_t comp_size)
{
	GP_IO *new = malloc(sizeof(GP_IO) + sizeof(struct priv));
	struct priv *priv;
	int err, ret;

	if (!new) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	priv = GP_IO_PRIV(new);

	priv->io = io;
	priv->comp_avail = comp_size;
	priv->comp_size  = comp_size;
	priv->bytes_read = 0;
        priv->crc = crc32(0, NULL, 0);
	priv->io_start = GP_IOTell(io);
	priv->eos = 0;

	priv->strm.zalloc = Z_NULL;
	priv->strm.zfree = Z_NULL;
	priv->strm.opaque = Z_NULL;
	priv->strm.avail_in = Z_NULL;
	priv->strm.next_in = Z_NULL;

	ret = inflateInit2(&priv->strm, -15);
	if (ret != Z_OK) {
		//TODO better err and message
		GP_DEBUG(1, "inflateInit() failed: %i", ret);
		err = EIO;
		goto err1;
	}

	//INIT IO
	new->Close = zlib_close;
	new->Read  = zlib_read;
	new->Write = NULL;
	new->Seek = zlib_seek;

	GP_DEBUG(1, "Initialized ZlibIO (%p)", new);

	return new;
err1:
	free(new);
	errno = err;
	return NULL;
}

#else

GP_IO *GP_IOZlib(GP_IO *io, size_t comp_size)
{
	errno = ENOSYS;
	return NULL;
}

int GP_IOZlibReset(GP_IO *io, GP_IO *sub_io, size_t comp_size)
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_ZLIB */
