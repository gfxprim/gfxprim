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

#ifndef LOADERS_GP_TIFF_H
#define LOADERS_GP_TIFF_H

#include "loaders/GP_Loader.h"

/*
 * Reads first image in TIFF from an IO stream.
 *
 * Returns newly allocated context cotaining the loaded image or in case of
 * failure NULL and errno is set.
 */
GP_Context *GP_ReadTIFF(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads fist image in TIFF from a file.
 */
GP_Context *GP_LoadTIFF(const char *src_path, GP_ProgressCallback *callback);

/*
 * Saves TIFF.
 */
int GP_SaveTIFF(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback);

/*
 * Looks for TIFF file signature. Returns non-zero if found.
 */
int GP_MatchTIFF(const void *buf);

extern GP_Loader GP_TIFF;

#endif /* LOADERS_GP_TIFF_H */
