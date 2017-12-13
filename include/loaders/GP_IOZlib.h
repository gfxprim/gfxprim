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

   Zlib decompression stream.

  */

#ifndef LOADERS_GP_IO_ZLIB_H
#define LOADERS_GP_IO_ZLIB_H

#include <loaders/GP_IO.h>

/*
 * Create an Zlib RAW inflate stream on the top of the existing I/O stream.
 *
 * The stream will read up to comp_size bytes from the parent I/O.
 *
 * If comp_size is 0, no limit on number bytes from the parent stream is set.
 * However if end of compressed stream is reached the last read will attempt to
 * seek back by the number of extra buffered bytes.
 */
gp_io *gp_io_zlib(gp_io *io, size_t comp_size);

/*
 * Repurposes existing Zlib stream for new decompression.
 *
 * Returns zero on success. Returns non-zero on failure and errno is set.
 */
int gp_io_zlib_reset(gp_io *io, gp_io *sub_io, size_t comp_size);

#endif /* LOADERS_GP_IO_ZLIB_H */
