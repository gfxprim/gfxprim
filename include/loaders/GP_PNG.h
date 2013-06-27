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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   PNG support using libpng library.

  */

#ifndef LOADERS_GP_PNG_H
#define LOADERS_GP_PNG_H

#include "core/GP_ProgressCallback.h"
#include "core/GP_Context.h"

#include "GP_MetaData.h"

/*
 * The possible errno values:
 *
 * - Anything FILE operation may return (fopen(), fclose(), fseek(), ...).
 * - EIO for png_read()/png_write() failure
 * - ENOSYS for not implemented bitmap format
 * - ENOMEM from malloc()
 * - EILSEQ for wrong image signature/data
 * - ECANCELED when call was aborted from callback
 */

/*
 * Opens up file and checks signature. Upon successful return (zero is
 * returned) the file position would be set to eight bytes (exactly after the
 * PNG signature).
 */
int GP_OpenPNG(const char *src_path, FILE **f);

/*
 * Reads PNG from an open FILE. Expects the file position set after the eight
 * bytes PNG signature.
 *
 * Upon succesfull return pointer to newly allocated context is returned.
 * Otherwise NULL is returned and errno is filled.
 */
GP_Context *GP_ReadPNG(FILE *f, GP_ProgressCallback *callback);

/*
 * Does both GP_OpenPNG and GP_ReadPNG at once.
 */
GP_Context *GP_LoadPNG(const char *src_path, GP_ProgressCallback *callback);

/*
 * Loads png meta-data.
 */
int GP_ReadPNGMetaData(FILE *f, GP_MetaData *data);
int GP_LoadPNGMetaData(const char *src_path, GP_MetaData *data);

/*
 * Saves PNG to a file. Zero is returned on succes. Upon failure non-zero is
 * returned and errno is filled accordingly.
 */
int GP_SavePNG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * Match PNG signature.
 */
int GP_MatchPNG(const void *buf);

#endif /* LOADERS_GP_PNG_H */
