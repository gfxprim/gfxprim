// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_io_zlib.h
  * @brief Zlib decompression stream.
  */

#ifndef LOADERS_GP_IO_ZLIB_H
#define LOADERS_GP_IO_ZLIB_H

#include <loaders/gp_io.h>

/**
 * @brief Creates a Zlib RAW inflate stream on the top of the existing I/O stream.
 *
 * @param io An I/O.
 * @param comp_size The size of the compressed data in the I/O. The library
 *                  will not attempt to read more bytes than this. Setting
 *                  comp_bytes to 0 means unlimited.
 * @return A newly allocated and initialize zlib I/O or NULL in a case of a
 *         failure and errno is set.
 */
gp_io *gp_io_zlib(gp_io *io, size_t comp_size);

/**
 * @brief Repurposes existing Zlib stream for a new decompression.
 *
 * @param io An I/O.
 * @param sub_io A zlib I/O previously allocated by gp_io_zlib().
 * @param comp_size The size of compressed data. See gp_io_zlib() for complete
 *                  description.
 * @return Zero on success. Returns non-zero on failure and errno is set.
 */
int gp_io_zlib_reset(gp_io *io, gp_io *sub_io, size_t comp_size);

#endif /* LOADERS_GP_IO_ZLIB_H */
