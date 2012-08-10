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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Core include file for loaders API.

  */

#ifndef LOADERS_GP_LOADERS_H
#define LOADERS_GP_LOADERS_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

#include "GP_PBM.h"
#include "GP_PGM.h"
#include "GP_PPM.h"

#include "GP_BMP.h"
#include "GP_PNG.h"
#include "GP_JPG.h"
#include "GP_GIF.h"

#include "GP_TmpFile.h"

#include "GP_MetaData.h"

/*
 * Tries to load image accordingly to the file extension.
 *
 * If operation fails NULL is returned and errno is filled.
 */
GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback);

/*
 * Loads image Meta Data (if possible).
 */
int GP_LoadMetaData(const char *src_path, GP_MetaData *data);

/*
 * Simple saving function, the image format is matched by file extension.
 *
 * Retruns zero on succes.
 * 
 * On failure non-zero is returned.
 *
 * When file type wasn't recognized by extension or if support for requested
 * image format wasn't compiled in non-zero is returned and errno is set to
 * ENOSYS.
 * 
 * The resulting errno may also be set to any possible error from fopen(3), open(3),
 * write(3), fwrite(3), seek(3), etc..
 */
int GP_SaveImage(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback);

#endif /* LOADERS_GP_LOADERS_H */
