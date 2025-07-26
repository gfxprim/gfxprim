// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_io.h
  * @brief I/O abstraction for gfxprim loaders.
  */

#ifndef LOADERS_GP_IO_H
#define LOADERS_GP_IO_H

#include <stdint.h>
#include <sys/types.h>
#include <core/gp_compiler.h>
#include <utils/gp_seek.h>
#include <loaders/gp_types.h>

/**
 * @brief An I/O abstraction.
 */
struct gp_io {
	/**
	 * @brief A read() callback.
	 *
	 * @param self An I/O.
	 * @param buf A buffer to read to.
	 * @param size A size of the buffer.
	 *
	 * @return A number of bytes that were read <= size or -1 on error.
	 */
	ssize_t (*read)(gp_io *self, void *buf, size_t size);
	/**
	 * @brief A write() callback.
	 *
	 * @param self An I/O.
	 * @param buf A buffer to write to.
	 * @param size A size of the buffer.
	 *
	 * @return A number of bytes that were written <= size or -1 on error.
	 */
	ssize_t (*write)(gp_io *self, const void *buf, size_t size);
	/**
	 * @brief A seek() callback.
	 *
	 * @param off An offset in the file.
	 * @param whence Defines to what the offset is relative to.
	 *
	 * @return A new offset in the file or (off_t)-1 on a failure.
	 */
	off_t (*seek)(gp_io *self, off_t off, enum gp_seek_whence whence);
	/**
	 * @brief A close() callback.
	 *
	 * Finalizes writes, frees memory.
	 *
	 * @param self An I/O.
	 *
	 * @return Zero on success -1 on a failure.
         */
	int (*close)(gp_io *self);

	/**
	 * @brief A mark to store offset to by the gp_io_mark().
	 */
	off_t mark;

	char priv[];
};

#define GP_IO_PRIV(io) ((void *)(io)->priv)

/**
 * @brief A wrapper for gp_io::read().
 *
 * @param self An I/O.
 * @param buf A buffer to read to.
 * @param size A size of the buffer.
 *
 * @return A number of bytes that were read <= size or -1 on error.
 */
static inline ssize_t gp_io_read(gp_io *self, void *buf, size_t size)
{
	return self->read(self, buf, size);
}

/**
 * @brief A wrapper for gp_io::write().
 *
 * @param self An I/O.
 * @param buf A buffer to write to.
 * @param size A size of the buffer.
 *
 * @return A number of bytes that were written <= size or -1 on error.
 */
static inline ssize_t gp_io_write(gp_io *self, const void *buf, size_t size)
{
	return self->write(self, buf, size);
}

/**
 * @brief A wrapper for gp_io::seek().
 *
 * @param self An I/O.
 * @param off An offset in the file.
 * @param whence Defines to what the offset is relative to.
 *
 * @return A new offset in the file or (off_t)-1 on a failure.
 */
static inline off_t gp_io_seek(gp_io *self, off_t off, enum gp_seek_whence whence)
{
	return self->seek(self, off, whence);
}

/**
 * @brief A wrapper for gp_io::close().
 *
 * Finalizes writes, frees memory.
 *
 * @param self An I/O.
 *
 * @return Zero on success -1 on a failure.
 */
static inline int gp_io_close(gp_io *self)
{
	return self->close(self);
}

/**
 * @brief A wrapper around gp_io::write() for a single byte.
 *
 * @param self An I/O.
 * @param b A byte to write into the I/O.
 *
 * @return Zero on success, non-zero on failure.
 */
static inline int gp_io_putb(gp_io *self, char b)
{
	return self->write(self, &b, 1) != 1;
}

/**
 * @brief A wrapper around gp_io::read() for a single byte.
 * @param self An I/O.
 *
 * @return A byte read from I/O or -1 on a failure.
 */
static inline int gp_io_getb(gp_io *self)
{
	unsigned char c;

	if (self->read(self, &c, 1) != 1)
		return -1;

	return c;
}

/**
 * @brief Returns current offset.
 *
 * @param self An I/O.
 *
 * @return Current offset in I/O.
 */
static inline off_t gp_io_tell(gp_io *self)
{
	return self->seek(self, 0, GP_SEEK_CUR);
}

/**
 * @brief Rewinds to start of the I/O stream.
 *
 * @param self An I/O.
 *
 * @return The new offset (i.e. zero) or (off_t)-1 on a failure.
 */
static inline off_t gp_io_rewind(gp_io *self)
{
	return self->seek(self, 0, GP_SEEK_SET);
}

/**
 * @brief Reads bytes but does not remove then from I/O.
 *
 * @param self An I/O.
 * @param buf A buffer to read to.
 * @param size A size of the buffer.
 *
 * //TODO: fix up the semantics
 *
 * @return -1 on a failure.
 */
static inline off_t gp_io_peek(gp_io *self, void *buf, size_t size)
{
	off_t cur_off = gp_io_tell(self);

	if (gp_io_read(self, buf, size) != (ssize_t)size)
		return -1;

	return gp_io_seek(self, cur_off, GP_SEEK_SET);
}

/**
 * @brief Returns I/O stream size.
 *
 * @param self An I/O.
 *
 * @return A I/O size or (off_t)-1 in case that gp_io_seek(io, 0, SEEK_END) is
 *         not possible.
 */
off_t gp_io_size(gp_io *self);

/**
 * @brief Fills whole buffer or returns error.
 *
 * This is a gp_io_read() call with retries that attempts to fill the buffer as
 * long as the underlying read returns any data. It will fail for example if we
 * request more data than there is in a file.
 *
 * @param self An I/O.
 * @param buf A buffer to read to.
 * @param size A size of the buffer.
 *
 * @return zero on success non-zero on failure.
 */
int gp_io_fill(gp_io *self, void *buf, size_t size);

/**
 * @brief Writes whole buffer or retuns error.
 *
 * This is a gp_io_write() call with retries that attempts to write whole
 * buffer as long as the underlying write writes any data. It will fail for
 * example if we attempt to write any data into a file on a full filesystem.
 *
 * @param self An I/O.
 * @param buf A buffer to write to.
 * @param size A size of the buffer.
 *
 * @return Zero on succes non-zero on a failure.
 */
int gp_io_flush(gp_io *self, const void *buf, size_t size);

/**
 * @brief Mark operation.
 */
enum gp_io_mark_op {
	/** @brief Puts a mark into an I/O. */
	GP_IO_MARK,
	/** @brief Rewinds the I/O to a mark. */
	GP_IO_REWIND,
};

/**
 * @brief Marks a current position or returns to mark in I/O stream.
 *
 * Only one mark can be placed on an I/O, subsequent calls to gp_io_mark() with
 * GP_IO_MARK rewrite previous mark.
 *
 * @param self An I/O.
 * @param op An operation either GP_IO_MARK or GP_IO_REWIND.
 *
 * @return Zero on success, -1 on a failure.
 */
int gp_io_mark(gp_io *self, enum gp_io_mark_op op);

/**
 * @brief Formatted binary read and write types.
 *
 * This interface is a bit similar to a printf and scanf but instead of format
 * string we have an array of 16bit unsigned integers describing the data.
 */
enum gp_io_fmt_types {
	/**
	 * @brief A constant byte in lower half of the 16bit integer.
	 *
	 * Since this is defined as a zero values less than or equal to 0xff
	 * are interpreted as constant bytes and matched or written exactly.
	 */
	GP_IO_CONST = 0x0000,
	/** @brief Reads or writes a single byte. */
	GP_IO_BYTE = 0x0100,
	/**
	 * @brief Reads or writes two bytes in a little endian order.
	 *
	 * The bytes are read or written into the corresponding pionter as a
	 * 16bit integer in a machine endinanity.
	 */
	GP_IO_L2 = 0x0200,
	/**
	 * @brief Reads or writes four bytes in a little endian order.
	 *
	 * The bytes are read or written into the corresponding pionter as a
	 * 32bit integer in a machine endinanity.
	 */
	GP_IO_L4 = 0x0300,
	/**
	 * @brief Reads or writes two bytes in a big endian order.
	 *
	 * The bytes are read or written into the corresponding pionter as a
	 * 16bit integer in a machine endinanity.
	 */
	GP_IO_B2 = 0x0400,
	/**
	 * @brief Reads or writes four bytes in a big endian order.
	 *
	 * The bytes are read or written into the corresponding pionter as a
	 * 32bit integer in a machine endinanity.
	 */
	GP_IO_B4 = 0x0500,
	/** @brief Pointer to byte array, size in lower half */
	GP_IO_ARRAY = 0x0600,
	/** @brief Ignore bytes on read, size in lower half. */
	GP_IO_IGN = 0x0700,
	/** @brief Ignore single byte on read. */
	GP_IO_I1 = GP_IO_IGN | 1,
	/** @brief Ignore two bytes on read. */
	GP_IO_I2 = GP_IO_IGN | 2,
	/** @brief Ignore three bytes on read. */
	GP_IO_I3 = GP_IO_IGN | 3,
	/** @brief Ignore four bytes on read. */
	GP_IO_I4 = GP_IO_IGN | 4,
	/**
	 * @brief A photoshop pascal string
	 *
	 * First byte stores size and string is padded to even number bytes.
	 *
	 * The lower half stores passed buffer size.
	 *
	 * TODO: Unfinished
	 */
	GP_IO_PPSTR = 0x0800,
	/**
	 * @brief End of the types array.
	 *
	 * This is terminating entry for the array.
	 */
	GP_IO_END = 0xff00,
};

#define GP_IO_TYPE_MASK 0xff00

/**
 * @brief Parses a data accodingly to the array of types.
 *
 * Example usage:
 * @code
 *	uint16_t ihdr_size, width, height;
 *
 *	const uint16_t png_header[] = {
 *		0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n',
 *		GP_IO_B4, // IHDR size
 *		'I', 'H', 'D', 'R',
 *		GP_IO_B4, // Width
 *		GP_IO_B4, // Height
 *		GP_IO_END,
 *	};
 *
 *	int ret = gp_io_readf(io, png_header, &ihdr_size, &width, &height);
 *
 *	if (ret != ARRAY_SIZE(png_header) - 1) {
 *		//error
 *	}
 *
 *	printf("PNG image size %ux%u\n", (unsigned int)width, (unsigned int)height);
 * @endcode
 *
 * @param self A readable I/O.
 * @param types An array of enum #gp_io_fmt_types terminated by GP_IO_END
 *              describing the values to be read.
 * @param ... Pointers to data describes in the types array.
 *
 * @return A number of read types or -1 in a case of a failure.
 */
int gp_io_readf(gp_io *self, const uint16_t *types, ...);

/**
 * @brief Writes a data accodingly to the array of types.
 *
 * Example usage:
 * @code
 *	uint16_t bitmap_header[] = {
 *		'B', 'M',               // signature
 *		GP_IO_L4,               // offset to pixels
 *		0x00, 0x00, 0x00, 0x00, // reserved
 *		GP_IO_L4,               // file size
 *		GP_IO_END,
 *	};
 *
 *	if (gp_io_writef(io, bitmap_header, file_size, pixel_offset))
 *		return EIO;
 * @endcode
 *
 * @param self A writeable I/O.
 * @param types An array of enum #gp_io_fmt_types terminated by GP_IO_END
 *              describing the values to be written.
 * @param ... Pointers to data describes in the types array.
 *
 * @return A zero on success.
 */
int gp_io_writef(gp_io *self, const uint16_t *types, ...);

/**
 * @brief Printf like function.
 *
 * @param self An I/O.
 * @param fmt A printf()-like format.
 * @param ... A printf()-like parameters.
 *
 * @return Zero on success, non-zero on failure.
 */
int gp_io_printf(gp_io *self, const char *fmt, ...)
    GP_FMT_PRINTF(2, 3);

/**
 * @brief Reads a single 32bit big endian integer.
 *
 * The integer is read as a big endian from the I/O and stored into the
 * ppointer in the machine endianity.
 *
 * @param self A readable I/O.
 * @param val A pointer to store the integer value into.
 *
 * @return Zero on success non-zero otherwise.
 */
int gp_io_read_b4(gp_io *self, uint32_t *val);

/**
 * @brief Reads a single 16bit big endian integer.
 *
 * The integer is read as a big endian from the I/O and stored into the
 * ppointer in the machine endianity.
 *
 * @param self A readable I/O.
 * @param val A pointer to store the integer value into.
 *
 * @return Zero on success non-zero otherwise.
 */
int gp_io_read_b2(gp_io *self, uint16_t *val);

/**
 * @brief A file mode for gp_io_file().
 */
enum gp_io_file_mode {
	/** @brief Opens file read only. */
	GP_IO_RDONLY = 0x00,
	/** @brief Opens file write only and truncates it if it exists. */
	GP_IO_WRONLY = 0x01,
	/** @brief Opens a file read/write. */
	GP_IO_RDWR = 0x02,
};

/**
 * @brief Creates I/O from a file.
 *
 * The I/O is not buffered, calls to gp_io_read() and gp_io_write() are
 * directly translated into read() and write() calls.
 *
 * @param path A filesystem path.
 * @param mode Open the I/O read-only, write-only or read-write. Write-only I/O
 *             files are truncated.
 *
 * @return A newly allocated I/O or NULL on error and errno is set.
 */
gp_io *gp_io_file(const char *path, enum gp_io_file_mode mode);

/**
 * @brief Creates a readable I/O from a memory buffer.
 *
 * @param buf A buffer with data.
 * @param size A size of the data buffer.
 * @param free A callback that is called with the buf pointer as a parameter on
 *             gp_io_close(). If no callback is needed NULL should be passed.
 *
 * @return A newly allocated and initialized I/O or NULL in a case of a allocation failure.
 */
gp_io *gp_io_mem(void *buf, size_t size, void (*free)(void *));

/**
 * @brief Create a read-only sub I/O from a readable I/O.
 *
 * The sub I/O starts at current offset in the parent I/O (which is also point
 * where gp_io_tell() for the new I/O will return zero) and continues for
 * up to size bytes in the parent I/O.
 *
 * WARNING: If you combine reading/writing in the Sub I/O and parent I/O the
 *          result is undefined.
 *
 * @param self A parent I/O.
 * @param size A size limit of the sub I/O.
 *
 * @return A newly allocated and initialized sub I/O or a NULL in a case of
 *         allocation failure.
 */
gp_io *gp_io_sub_io(gp_io *self, size_t size);

/**
 * @brief Creates a writeable buffered I/O on the top of the existing I/O.
 *
 * @param self A writeable I/O.
 * @param bsize A buffer size. Passing zero as bsize select default buffer
 *              size.
 *
 * @return A newly allocated I/O or NULL in a case of an allocation failure.
 */
gp_io *gp_io_wbuffer(gp_io *self, size_t bsize);

#endif /* LOADERS_GP_IO_H */
