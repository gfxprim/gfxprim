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
   
   JPG support using jpeg library.

  */

#ifndef LOADERS_GP_JPG_H
#define LOADERS_GP_JPG_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * Opens up file and checks signature.
 */
GP_RetCode GP_OpenJPG(const char *src_path, FILE **f);

/*
 * Reads JPG from an open FILE. Expects the file possition set after the eight
 * bytes JPG signature.
 */
GP_RetCode GP_ReadJPG(FILE *f, GP_Context **res,
                      GP_ProgressCallback *callback);

/*
 * Loads a JPG file into GP_Context. The Context is newly allocated.
 */
GP_RetCode GP_LoadJPG(const char *src_path, GP_Context **res,
                      GP_ProgressCallback *callback);

/*
 * Saves JPG to a file.
 */
GP_RetCode GP_SaveJPG(const char *dst_path, const GP_Context *src,
                      GP_ProgressCallback *callback);


#endif /* LOADERS_GP_JPG_H */
