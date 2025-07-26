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
 * @brief Create a Zlib RAW inflate stream on the top of the existing I/O stream.
 *
 * The stream will read up to comp_size bytes from the parent I/O.
 *
 * If comp_size is 0, no limit on number bytes from the parent stream is set.
 * However if end of compressed stream is reached the last read will attempt to
 * seek back by the number of extra buffered bytes.
 */
gp_io *gp_io_zlib(gp_io *io, size_t comp_size);

/**
 * @brief Repurposes existing Zlib stream for new decompression.
 *
 * @return Zero on success. Returns non-zero on failure and errno is set.
 */
int gp_io_zlib_reset(gp_io *io, gp_io *sub_io, size_t comp_size);

#endif /* LOADERS_GP_IO_ZLIB_H */
