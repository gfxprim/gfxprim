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

   JPEG 2000 support using openjpeg library.

  */

#ifndef LOADERS_GP_JP2_H
#define LOADERS_GP_JP2_H

#include "loaders/GP_Loader.h"

int GP_ReadJP2Ex(GP_IO *io, GP_Pixmap **rimg, GP_DataStorage *storage,
                 GP_ProgressCallback *callback);

int GP_LoadJP2Ex(const char *src_path, GP_Pixmap **img,
		 GP_DataStorage *storage, GP_ProgressCallback *callback);

/*
 * Reads a JPEG2000 from an IO stream.
 */
GP_Pixmap *GP_ReadJP2(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads a JPEG2000 image from a file.
 */
GP_Pixmap *GP_LoadJP2(const char *src_path, GP_ProgressCallback *callback);

/*
 * Looks for JPEG2000 file signature. Returns non-zero if found.
 */
int GP_MatchJP2(const void *buf);

extern GP_Loader GP_JP2;

#endif /* LOADERS_GP_JP2_H */
