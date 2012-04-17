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
   
   GIF support using giflib library.

  */

#ifndef LOADERS_GP_GIF_H
#define LOADERS_GP_GIF_H

#include "core/GP_ProgressCallback.h"
#include "core/GP_Context.h"

/*
 * The possible errno values:
 *
 * - EIO for read/write failure
 * - ENOSYS for not implemented bitmap format
 * - ENOMEM from malloc()
 * - EILSEQ for wrong image signature/data
 * - ECANCELED when call was aborted from callback
 */

/*
 * Opens up the Gif image and checks signature.
 * Returns zero on success.
 */
int GP_OpenGIF(const char *src_path, void **f);

/*
 * Reads first image found in GIF container.
 */
GP_Context *GP_ReadGIF(void *f, GP_ProgressCallback *callback);

/*
 * Does both GP_OpenGIF and GP_ReadGIF at once.
 */
GP_Context *GP_LoadGIF(const char *src_path, GP_ProgressCallback *callback);

#endif /* LOADERS_GP_GIF_H */
