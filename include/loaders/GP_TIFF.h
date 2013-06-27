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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef LOADERS_GP_TIFF_H
#define LOADERS_GP_TIFF_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * The possible errno values:
 *
 * - Anything FILE operation may return (fopen(), fclose(), fseek(), ...).
 * - EIO for fread()/fwrite() failure
 * - ENOSYS for not implemented bitmap format
 * - ENOMEM from malloc()
 * - EILSEQ for wrong image signature/data
 * - ECANCELED when call was aborted from callback
 */

/*
 * Opens up a bmp file, checks signature, parses metadata.
 *
 * The file, width, height and pixel type are filled upon succcessful return.
 *
 * Upon failure, non zero return value is returned and errno is filled.
 */
int GP_OpenTIFF(const char *src_path, void **t);

/*
 * Reads a TIFF from a opened file.
 *
 * Upon successful return, context to store bitmap is allocated and image is
 * loaded.
 *
 * Upon failure NULL is returned and errno is filled.
 */
GP_Context *GP_ReadTIFF(void *t, GP_ProgressCallback *callback);

/*
 * Does both GP_OpenTIFF and GP_ReadTIFF.
 */
GP_Context *GP_LoadTIFF(const char *src_path, GP_ProgressCallback *callback);

/*
 * Match TIFF signature.
 */
int GP_MatchTIFF(const void *buf);

#endif /* LOADERS_GP_TIFF_H */
