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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*
   
   PNG support using libpng library.

  */

#ifndef LOADERS_GP_PNG_H
#define LOADERS_GP_PNG_H

#include "core/GP_ProgressCallback.h"
#include "core/GP_Context.h"

/*
 * Opens up file and checks signature. Upon successful return the file
 * possition would be set to eight bytes (exactly after the PNG signature).
 */
GP_RetCode GP_OpenPNG(const char *src_path, FILE **f);

/*
 * Reads PNG from an open FILE. Expects the file possition set after the eight
 * bytes PNG signature.
 */
GP_RetCode GP_ReadPNG(FILE *f, GP_Context **res,
                      GP_ProgressCallback *callback);

/*
 * Loads a PNG file into GP_Context. The Context is newly allocated.
 */
GP_RetCode GP_LoadPNG(const char *src_path, GP_Context **res,
                      GP_ProgressCallback *callback);

/*
 * Saves PNG to a file.
 */
GP_RetCode GP_SavePNG(const char *dst_path, const GP_Context *src,
                      GP_ProgressCallback *callback);

#endif /* LOADERS_GP_PNG_H */
