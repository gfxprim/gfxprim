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
   
   Experimental Paint Shop Pro image loader.

  */

#ifndef LOADERS_GP_PSP_H
#define LOADERS_GP_PSP_H

#include "core/GP_ProgressCallback.h"
#include "core/GP_Context.h"

/*
 * The possible errno values:
 *
 * - EIO for read/write failure
 * - ENOSYS for not implemented bitmap format
 * - ENOMEM from malloc()
 * - EILSEQ for wrong image signature/data
 * - ECANCELED when call was aborted from callback
 */

/*
 * Opens up the PSP image and checks signature.
 * Returns zero on success.
 */
int GP_OpenPSP(const char *src_path, FILE **f);

/*
 * Reads image from PSP format.
 */
GP_Context *GP_ReadPSP(FILE *f, GP_ProgressCallback *callback);

/*
 * Does both GP_OpenPSP and GP_ReadPSP at once.
 */
GP_Context *GP_LoadPSP(const char *src_path, GP_ProgressCallback *callback);

/*
 * Match PSP signature.
 */
int GP_MatchPSP(const void *buf);

#endif /* LOADERS_GP_PSP_H */
