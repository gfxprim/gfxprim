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

 /*

   I/O abstraction for gfxprim loaders.

  */

#ifndef LOADERS_GP_IO_H
#define LOADERS_GP_IO_H

#include <stdint.h>
#include <sys/types.h>

/*
 * Values are 1:1 with constants passed to lseek()
 */
enum gp_io_whence {
	GP_IO_SEEK_SET = 0,
	GP_IO_SEEK_CUR = 1,
	GP_IO_SEEK_END = 2,
};

typedef struct gp_io gp_io;

struct gp_io {
	ssize_t (*read)(gp_io *self, void *buf, size_t size);
	ssize_t (*write)(gp_io *self, void *buf, size_t size);
	off_t (*seek)(gp_io *self, off_t off, enum gp_io_whence whence);
	int (*close)(gp_io *self);

	off_t mark;
	char priv[];
};

#define GP_IO_PRIV(io) ((void *)(io)->priv)

/*
 * Just inline wrappers.
 */
static inline ssize_t gp_io_read(gp_io *io, void *buf, size_t size)
{
	return io->read(io, buf, size);
}

static inline ssize_t gp_io_write(gp_io *io, void *buf, size_t size)
{
	return io->write(io, buf, size);
}

static inline int gp_io_close(gp_io *io)
{
	return io->close(io);
}

static inline off_t gp_io_seek(gp_io *io, off_t off, enum gp_io_whence whence)
{
	return io->seek(io, off, whence);
}

/*
 * PutC returns zero on success, non-zero on failure.
 */
static inline int gp_io_putc(gp_io *io, char c)
{
	return io->write(io, &c, 1) != 1;
}

/*
 * Returns current offset
 */
static inline off_t gp_io_tell(gp_io *io)
{
	return io->seek(io, 0, GP_IO_SEEK_CUR);
}

/*
 * Rewinds to start of the I/O stream.
 */
static inline off_t gp_io_rewind(gp_io *io)
{
	return io->seek(io, 0, GP_IO_SEEK_SET);
}

/*
 * Returns I/O stream size.
 *
 * May return (off_t)-1 in case that gp_io_SEEK_END is not possible.
 */
off_t gp_io_size(gp_io *io);

/*
 * Like a Read but either fills whole buffer or returns error.
 *
 * Returns zero on success non-zero on failure.
 */
int gp_io_fill(gp_io *io, void *buf, size_t size);

/*
 * Like Write but either writes whole buffer or retuns error.
 *
 * Returns zero on succes non-zero on failure.
 */
int gp_io_flush(gp_io *io, void *buf, size_t size);

/*
 * Marks a current position, returns to mark in I/O stream.
 */
enum gp_io_mark_types {
	GP_IO_MARK,
	GP_IO_REWIND,
};

int gp_io_mark(gp_io *self, enum gp_io_mark_types type);

/*
 * Formatted read.
 */
enum gp_io_fmt_types {
	/* Constant byte in lower half */
	GP_IO_CONST = 0x0000,
	/* Pointer to one byte */
	GP_IO_BYTE = 0x0100,
	/* Pointer to byte integer in litte endian */
	GP_IO_L2 = 0x0200,
	/* Poiter to four byte integer in litte endian */
	GP_IO_L4 = 0x0300,
	/* Pointer to two byte integer in big endian */
	GP_IO_B2 = 0x0400,
	/* Pointer to four byte integer in big endian */
	GP_IO_B4 = 0x0500,
	/* Pointer to byte array, size in lower half */
	GP_IO_ARRAY = 0x0600,
	/* Ignore bytes on read, size in lower half */
	GP_IO_IGN = 0x0700,
	GP_IO_I1 = GP_IO_IGN | 1,
	GP_IO_I2 = GP_IO_IGN | 2,
	GP_IO_I3 = GP_IO_IGN | 3,
	GP_IO_I4 = GP_IO_IGN | 4,
	/*
	 * Photoshop Pascal string
	 *
	 * first byte stores size and string is padded to even number bytes.
	 *
	 * The lower half stores passed buffer size.
	 *
	 * TODO: Unfinished
	 */
	GP_IO_PPSTR = 0x0800,
	/* End of the types array */
	GP_IO_END = 0xff00,
};

#define GP_IO_TYPE_MASK 0xff00

int gp_io_readf(gp_io *self, uint16_t *types, ...);

int gp_io_writef(gp_io *self, uint16_t *types, ...);

/*
 * Printf like function.
 *
 * Returns zero on success, non-zero on failure.
 */
int gp_io_printf(gp_io *self, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

/*
 * gp_io_readf wrappers for convinient reading of single value
 */
int gp_io_read_b4(gp_io *io, uint32_t *val);

int gp_io_read_b2(gp_io *io, uint16_t *val);

enum gp_io_file_mode {
	GP_IO_RDONLY = 0x00,
	GP_IO_WRONLY = 0x01,
	GP_IO_RDWR = 0x02,
};

/*
 * Creates I/O from a file. On error NULL is returned and errno is set.
 */
gp_io *gp_io_file(const char *path, enum gp_io_file_mode mode);

/*
 * Creates I/O from a memory buffer.
 *
 * If free is not NULL, it's called on buf pointer on gp_ioClose().
 */
gp_io *gp_io_mem(void *buf, size_t size, void (*free)(void *));

/*
 * Create a sub I/O from an I/O.
 *
 * The sub I/O starts at current offset in the parent I/O (which is also point
 * where gp_ioTell() for the new I/O will return zero) and continues for
 * maximally size bytes in the parent I/O. Reads at the end of the Sub I/O will
 * be truncated to the.
 *
 * WARNING: If you combine reading/writing in the Sub I/O and parent I/O the
 *          result is undefined.
 */
gp_io *gp_io_sub_io(gp_io *pio, size_t size);

/*
 * Creates a writeable buffered I/O on the top of the existing I/O.
 *
 * Passing zero as bsize select default buffer size.
 */
gp_io *gp_io_wbuffer(gp_io *pio, size_t bsize);

#endif /* LOADERS_gp_io_H */
