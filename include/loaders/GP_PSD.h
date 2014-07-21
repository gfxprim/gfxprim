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

   PSD image loader.

   The loader tries to read merged image (if present) and fallbacks to
   thumbnail. It may return NULL with errno untouched if neither merged image
   nor thumbnail was present (which seem to be really uncommon).

  */

#ifndef LOADERS_GP_PSD_H
#define LOADERS_GP_PSD_H

#include "loaders/GP_Loader.h"

int GP_ReadPSDEx(GP_IO *io, GP_Context **img,
		 GP_DataStorage *storage, GP_ProgressCallback *callback);

int GP_LoadPSDEx(const char *src_path, GP_Context **img,
		 GP_DataStorage *storage, GP_ProgressCallback *callback);

GP_Context *GP_ReadPSD(GP_IO *io, GP_ProgressCallback *callback);

GP_Context *GP_LoadPSD(const char *src_path, GP_ProgressCallback *callback);

int GP_MatchPSD(const void *buf);

/*
 * Looks for PSD file signature. Returns non-zero if found.
 */
extern GP_Loader GP_PSD;

#endif /* LOADERS_GP_PSD_H */
