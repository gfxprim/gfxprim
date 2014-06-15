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

#ifndef LOADERS_GP_PNM_H
#define LOADERS_GP_PNM_H

#include "loaders/GP_Loader.h"

/*
 * PBM Bitmap
 */
GP_Context *GP_ReadPBM(GP_IO *io, GP_ProgressCallback *callback);

GP_Context *GP_LoadPBM(const char *src_path, GP_ProgressCallback *callback);

int GP_WritePBM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback);

int GP_SavePBM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PGM Graymap
 */
GP_Context *GP_ReadPGM(GP_IO *io, GP_ProgressCallback *callback);

GP_Context *GP_LoadPGM(const char *src_path,
                       GP_ProgressCallback *callback);

int GP_WritePGM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback);

int GP_SavePGM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PPM Pixmap
 */
GP_Context *GP_ReadPPM(GP_IO *io, GP_ProgressCallback *callback);

GP_Context *GP_LoadPPM(const char *src_path, GP_ProgressCallback *callback);

int GP_WritePPM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback);

int GP_SavePPM(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback);

/*
 * PNM Anymap (All of above)
 */
GP_Context *GP_ReadPNM(GP_IO *io, GP_ProgressCallback *callback);

GP_Context *GP_LoadPNM(const char *src_path, GP_ProgressCallback *callback);

int GP_WritePNM(const GP_Context *src, GP_IO *io,
                GP_ProgressCallback *callback);

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

extern GP_Loader GP_PBM;
extern GP_Loader GP_PGM;
extern GP_Loader GP_PPM;
extern GP_Loader GP_PNM;

#endif /* LOADERS_GP_PNM_H */
