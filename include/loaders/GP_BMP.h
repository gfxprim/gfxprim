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

#ifndef LOADERS_GP_BMP_H
#define LOADERS_GP_BMP_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"
#include "loaders/GP_IO.h"

/*
 * Reads a BMP from an IO stream.
 *
 * Returns newly allocated context cotaining the loaded image or in case of
 * failure NULL and errno is set.
 */
GP_Context *GP_ReadBMP(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads a BMP image from a file.
 */
GP_Context *GP_LoadBMP(const char *src_path, GP_ProgressCallback *callback);

/*
 * Writes a BMP to an IO Stream.
 *
 * Returns zero on success, non-zero on failure and errno is set.
 */
int GP_WriteBMP(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback);

/*
 * Saves BMP to a file. Zero is returned on succes. Upon failure non-zero is
 * returned and errno is filled accordingly.
 */
int GP_SaveBMP(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * Looks for BMP file signature. Returns non-zero if found.
 */
int GP_MatchBMP(const void *buf);

#endif /* LOADERS_GP_BMP_H */
