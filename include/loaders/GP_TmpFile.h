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

   This is interface for saving GP_Context into non-portable uncompressed file,
   which is usefull for caching GP_Context to disk.

  */

#ifndef LOADERS_GP_TMP_FILE_H
#define LOADERS_GP_TMP_FILE_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

/*
 * The possible errno values:
 *
 * - Anything FILE operation may return (fopen(), fclose(), fseek(), ...).
 * - EIO for fread()/fwrite() failure
 * - ENOMEM from malloc()
 * - ECANCELED when call was aborted from callback
 */

/*
 * Opens up and loads file.
 *
 * On failure NULL is returned and errno is set.
 */
GP_Context *GP_LoadTmpFile(const char *src_path, GP_ProgressCallback *callback);

/*
 * Saves context into a file. On failure non-zero is returned and errno is set.
 */
int GP_SaveTmpFile(const GP_Context *src, const char *dst_path,
                   GP_ProgressCallback *callback);

#endif /* LOADERS_GP_TMP_FILE_H */
