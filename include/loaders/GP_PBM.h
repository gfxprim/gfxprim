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

#ifndef LOADERS_GP_PBM_H
#define LOADERS_GP_PBM_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * Loads 1-bit Grayscale image from portable bitmap format.
 */
GP_Context *GP_LoadPBM(const char *src_path, GP_ProgressCallback *callback);

/*
 * Save 1-bit Grayscale image into portable bitmap format.
 *
 * On success zero is returned, otherwise non-zero is returned and errno is
 * filled:
 *
 * EINVAL - context pixel type was not 1 bit grayscale.
 *
 */
int GP_SavePBM(const GP_Context *src, const char *res_path,
               GP_ProgressCallback *callback);

#endif /* LOADERS_GP_PBM_H */
