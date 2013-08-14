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

 /*

   JPEG 2000 support using openjpeg library.

  */

#ifndef LOADERS_GP_JP2_H
#define LOADERS_GP_JP2_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * Opens up file and checks signature.
 */
int GP_OpenJP2(const char *src_path, FILE **f);

/*
 * Reads JP2 from an open FILE.
 */
GP_Context *GP_ReadJP2(FILE *f, GP_ProgressCallback *callback);

/*
 * Loads a JP2 file into GP_Context. The Context is newly allocated.
 */
GP_Context *GP_LoadJP2(const char *src_path, GP_ProgressCallback *callback);

/*
 * Match JP2 signature.
 */
int GP_MatchJP2(const void *buf);

#endif /* LOADERS_GP_JP2_H */
