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

   PCX image support.

  */

#ifndef LOADERS_GP_PCX_H
#define LOADERS_GP_PCX_H

#include "loaders/GP_Loader.h"

/*
 * Extended loading function.
 */
int GP_ReadPCXEx(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                 GP_ProgressCallback *callback);

int GP_LoadPCXEx(const char *src_path, GP_Context **img,
                 GP_DataStorage *storage, GP_ProgressCallback *callback);

/*
 * Reads a PCX from an IO stream.
 */
GP_Context *GP_ReadPCX(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Loads a PCX image from a file.
 */
GP_Context *GP_LoadPCX(const char *src_path, GP_ProgressCallback *callback);

/*
 * Match PCX signature.
 */
int GP_MatchPCX(const void *buf);

extern GP_Loader GP_PCX;

#endif /* LOADERS_GP_PCX_H */
