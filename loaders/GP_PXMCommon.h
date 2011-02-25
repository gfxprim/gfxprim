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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

 Common bytes to ascci and ascii to bytes functions.

 */

#ifndef GP_PXM_COMMON_H
#define GP_PXM_COMMON_H

#include <stdio.h>
#include "GP.h"

/*
 * Save context to ascii file.
 *
 * The pixel type is not checked here as these are internal funcitons.
 */
GP_RetCode GP_PXMSave1bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMSave2bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMSave4bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMSave8bpp(FILE *f, GP_Context *context);

/*
 * Load context from ascii file.
 */
GP_RetCode GP_PXMLoad1bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMLoad2bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMLoad4bpp(FILE *f, GP_Context *context);
GP_RetCode GP_PXMLoad8bpp(FILE *f, GP_Context *context);

#endif /* GP_PXM_COMMON_H */
