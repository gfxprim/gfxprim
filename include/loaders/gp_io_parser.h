// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_io_parser.h
  * @brief Commmon functions to parse data.
  */

#ifndef LOADERS_GP_IO_PARSER_H
#define LOADERS_GP_IO_PARSER_H

#include <loaders/gp_io.h>

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
 * @brief Reads unicode string stored in big endian utf16 format.
 *
 * @param io A readable I/O.
 * @param nbytes Number of characters, the number of bytes is twice the number
 *               of the characters for utf16.
 *
 * @return A newly allocated null terminated utf8 string or NULL in a case of a
 *         allocation failure.
 */
char *gp_io_read_b2_utf16(gp_io *io, size_t nchars);

#endif /* LOADERS_GP_IO_PARSER_H */
