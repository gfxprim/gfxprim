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

   I/O abstraction for GFXprim loaders.

  */

#ifndef LOADERS_GP_IO_H
#define LOADERS_GP_IO_H

#include <stdint.h>
#include <sys/types.h>

/*
 * Values are 1:1 with constants passed to lseek()
 */
enum GP_IOWhence {
	GP_IO_SEEK_SET = 0,
	GP_IO_SEEK_CUR = 1,
	GP_IO_SEEK_END = 2,
};

typedef struct GP_IO {
	ssize_t (*Read)(struct GP_IO *self, void *buf, size_t size);
	ssize_t (*Write)(struct GP_IO *self, void *buf, size_t size);
	off_t (*Seek)(struct GP_IO *self, off_t off, enum GP_IOWhence whence);
	int (*Close)(struct GP_IO *self);

//	void *(*Map)(struct GP_IO *self, size_t len, off_t off);
//	void (*UnMap)(struct GP_IO *self, void *addr, size_t len);

	off_t mark;
	char priv[];
} GP_IO;

#define GP_IO_PRIV(io) ((void *)(io)->priv)

/*
 * Just inline wrappers.
 */
static inline ssize_t GP_IORead(GP_IO *io, void *buf, size_t size)
{
	return io->Read(io, buf, size);
}

static inline ssize_t GP_IOWrite(GP_IO *io, void *buf, size_t size)
{
	return io->Write(io, buf, size);
}

static inline int GP_IOClose(GP_IO *io)
{
	return io->Close(io);
}

static inline off_t GP_IOSeek(GP_IO *io, off_t off, enum GP_IOWhence whence)
{
	return io->Seek(io, off, whence);
}

/*
 * PutC returns zero on success, non-zero on failure.
 */
static inline int GP_IOPutC(GP_IO *io, char c)
{
	return io->Write(io, &c, 1) != 1;
}

//static inline void *GP_IOMap(GP_IO *io, size_t len, off_t off)
//{
//	return io->Map(io, len, off);
//}

//static inline void *GP_IOUnMap(GP_IO *io, size_t len, off_t off)
//{
//	return io->UnMap(io, len, off);
//}

/*
 * Returns current offset
 */
static inline off_t GP_IOTell(GP_IO *io)
{
	return io->Seek(io, 0, GP_IO_SEEK_CUR);
}

/*
 * Rewinds to start of the I/O stream.
 */
static inline off_t GP_IORewind(GP_IO *io)
{
	return io->Seek(io, 0, GP_IO_SEEK_SET);
}

/*
 * Returns I/O stream size.
 *
 * May return (off_t)-1 in case that GP_IO_SEEK_END is not possible.
 */
off_t GP_IOSize(GP_IO *io);

/*
 * Like a Read but either fills whole buffer or returns error.
 *
 * Returns zero on success non-zero on failure.
 */
int GP_IOFill(GP_IO *io, void *buf, size_t size);

/*
 * Like Write but either writes whole buffer or retuns error.
 *
 * Returns zero on succes non-zero on failure.
 */
int GP_IOFlush(GP_IO *io, void *buf, size_t size);

/*
 * Marks a current position, returns to mark in I/O stream.
 */
enum GP_IOMarkTypes {
	GP_IO_MARK,
	GP_IO_REWIND,
};

int GP_IOMark(GP_IO *self, enum GP_IOMarkTypes type);

/*
 * Formatted read.
 *
 *
 */
enum GP_IOFTypes {
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

int GP_IOReadF(GP_IO *self, uint16_t *types, ...);

int GP_IOWriteF(GP_IO *self, uint16_t *types, ...);

/*
 * Printf like function.
 *
 * Returns zero on success, non-zero on failure.
 */
int GP_IOPrintF(GP_IO *self, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

/*
 * GP_IOReadF wrappers for convinient reading of single value
 */
int GP_IOReadB4(GP_IO *io, uint32_t *val);

int GP_IOReadB2(GP_IO *io, uint16_t *val);

enum GP_IOFileMode {
	GP_IO_RDONLY = 0x00,
	GP_IO_WRONLY = 0x01,
	GP_IO_RDWR = 0x02,
};

/*
 * Creates I/O from a file. On error NULL is returned and errno is set.
 */
GP_IO *GP_IOFile(const char *path, enum GP_IOFileMode mode);

/*
 * Creates I/O from a memory buffer.
 *
 * If free is not NULL, it's called on buf pointer on GP_IOClose().
 */
GP_IO *GP_IOMem(void *buf, size_t size, void (*free)(void *));

/*
 * Create a sub I/O from an I/O.
 *
 * The sub I/O starts at current offset in the parent I/O (which is also point
 * where GP_IOTell() for the new I/O will return zero) and continues for
 * maximally size bytes in the parent I/O. Reads at the end of the Sub I/O will
 * be truncated to the.
 *
 * WARNING: If you combine reading/writing in the Sub I/O and parent I/O the
 *          result is undefined.
 */
GP_IO *GP_IOSubIO(GP_IO *pio, size_t size);

/*
 * Creates a writeable buffered I/O on the top of the existing I/O.
 *
 * Passing zero as bsize select default buffer size.
 */
GP_IO *GP_IOWBuffer(GP_IO *pio, size_t bsize);

#endif /* LOADERS_GP_IO_H */
