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

 Common bytes to ascci and ascii to bytes functions.

 */

#ifndef LOADERS_GP_PXM_COMMON_H
#define LOADERS_GP_PXM_COMMON_H

#include <stdio.h>
#include "core/GP_Core.h"

/*
 * Save context to ascii file.
 *
 * The pixel type is not checked here as these are internal funcitons.
 */
int GP_PXMSave1bpp(FILE *f, GP_Context *context);
int GP_PXMSave2bpp(FILE *f, GP_Context *context);
int GP_PXMSave4bpp(FILE *f, GP_Context *context);
int GP_PXMSave8bpp(FILE *f, GP_Context *context);

/*
 * Load context from ascii file.
 */
int GP_PXMLoad1bpp(FILE *f, GP_Context *context);
int GP_PXMLoad2bpp(FILE *f, GP_Context *context);
int GP_PXMLoad4bpp(FILE *f, GP_Context *context);
int GP_PXMLoad8bpp(FILE *f, GP_Context *context);

/*
 * Loads image header, returns pointer to FILE* on success, fills image
 * metadata into arguments.
 */
FILE *GP_ReadHeaderPNM(const char *src_path, char *fmt,
                       uint32_t *w, uint32_t *h, uint32_t *depth);

FILE *GP_WriteHeaderPNM(const char *dst_path, char *fmt,
                        uint32_t w, uint32_t h, uint32_t depth);
                        

#endif /* LOADERS_GP_PXM_COMMON_H */
