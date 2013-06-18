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

#ifndef LOADERS_GP_PNM_H
#define LOADERS_GP_PNM_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * PBM Bitmap
 */
GP_Context *GP_LoadPBM(const char *src_path, GP_ProgressCallback *callback);

int GP_SavePBM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PGM Graymap
 */
GP_Context *GP_LoadPGM(const char *src_path,
                       GP_ProgressCallback *callback);

int GP_SavePGM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PPM Pixmap
 */
GP_Context *GP_LoadPPM(const char *src_path, GP_ProgressCallback *callback);

int GP_SavePPM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PNM Anymap (All of above)
 */
GP_Context *GP_LoadPNM(const char *src_path, GP_ProgressCallback *callback);

int GP_SavePNM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * ASCII or rawbits Bitmap.
 */
int GP_MatchPBM(const void *buf);

/*
 * ASCII or rawbits Graymap.
 */
int GP_MatchPGM(const void *buf);

/*
 * ASCII or rawbits Pixmap.
 */
int GP_MatchPPM(const void *buf);

/*
 * All of above.
 */
int GP_MatchPNM(const void *buf);

#endif /* LOADERS_GP_PNM_H */
