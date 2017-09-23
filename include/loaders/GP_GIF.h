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

   GIF support using giflib library.

  */

#ifndef LOADERS_GP_GIF_H
#define LOADERS_GP_GIF_H

#include "loaders/GP_Loader.h"

/*
 * Reads first image found in GIF container from an IO stream.
 *
 * Returns newly allocated pixmap cotaining the loaded image or in case of
 * failure NULL and errno is set.
 */
GP_Pixmap *GP_ReadGIF(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads first image found in GIF container from a file.
 */
GP_Pixmap *GP_LoadGIF(const char *src_path, GP_ProgressCallback *callback);

/*
 * Looks for GIF file signature. Returns non-zero if found.
 */
int GP_MatchGIF(const void *buf);

extern GP_Loader GP_GIF;

#endif /* LOADERS_GP_GIF_H */
