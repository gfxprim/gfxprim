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

   PNG support using libpng library.

  */

#ifndef LOADERS_GP_PNG_H
#define LOADERS_GP_PNG_H

#include "loaders/GP_Loader.h"

/*
 * Reads a PNG from an IO stream.
 *
 * Returns newly allocated context cotaining the loaded image or in case of
 * failure NULL and errno is set.
 */
GP_Context *GP_ReadPNG(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads a PNG image from a file.
 */
GP_Context *GP_LoadPNG(const char *src_path, GP_ProgressCallback *callback);

/*
 * Loads PNG meta-data.
 */
int GP_ReadPNGMetaData(GP_IO *io, GP_MetaData *data);
int GP_LoadPNGMetaData(const char *src_path, GP_MetaData *data);

/*
 * Saves PNG to a file. Zero is returned on succes. Upon failure non-zero is
 * returned and errno is filled accordingly.
 */
int GP_SavePNG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * Looks for PNG file signature. Returns non-zero if found.
 */
int GP_MatchPNG(const void *buf);

extern GP_Loader GP_PNG;

#endif /* LOADERS_GP_PNG_H */
