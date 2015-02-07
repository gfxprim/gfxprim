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

   EXIF parser.

  */

#ifndef LOADERS_GP_EXIF_H
#define LOADERS_GP_EXIF_H

#include "loaders/GP_DataStorage.h"
#include "loaders/GP_Loader.h"

/*
 * Reads EXIF from an readable I/O stream and stores the values into storage.
 */
int GP_ReadExif(GP_IO *io, GP_DataStorage *storage);

/*
 * Looks for EXIF file signature. Returns non-zero if found.
 */
int GP_MatchExif(const void *buf);

/*
 * Defines position to 0,0 coordinate.
 */
enum GP_EXIF_ORIENTATION {
	GP_EXIF_UPPER_LEFT = 1,
	GP_EXIF_LOWER_RIGHT = 3,
	GP_EXIF_UPPER_RIGHT = 6,
	GP_EXIF_LOWER_LEFT = 8,
};

#endif /* LOADERS_GP_EXIF_H */
