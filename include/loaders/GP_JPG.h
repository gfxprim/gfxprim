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
   
   JPG support using jpeg library.

  */

#ifndef LOADERS_GP_JPG_H
#define LOADERS_GP_JPG_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

#include "GP_MetaData.h"

/*
 * Opens up file and checks signature.
 */
int GP_OpenJPG(const char *src_path, FILE **f);

/*
 * Reads JPG from an open FILE. Expects the file position set after the eight
 * bytes JPG signature.
 */
GP_Context *GP_ReadJPG(FILE *f, GP_ProgressCallback *callback);

/*
 * Loads a JPG file into GP_Context. The Context is newly allocated.
 */
GP_Context *GP_LoadJPG(const char *src_path, GP_ProgressCallback *callback);

/*
 * Loads JPEG meta-data, called markers in JPEG terminology.
 */
int GP_ReadJPGMetaData(FILE *f, GP_MetaData *data);
int GP_LoadJPGMetaData(const char *src_path, GP_MetaData *data);

/*
 * Saves JPG to a file.
 */
int GP_SaveJPG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * Match JPG signature.
 */
int GP_MatchJPG(const void *buf);

#endif /* LOADERS_GP_JPG_H */
