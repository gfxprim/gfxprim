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

   Core include file for loaders API.

  */

#ifndef LOADERS_GP_LOADER_H
#define LOADERS_GP_LOADER_H

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"
#include "loaders/GP_IO.h"
#include "loaders/GP_MetaData.h"


/*
 * Reads an image from a IO stream.
 *
 * The image format is matched from the file signature (first few bytes of the
 * IO stream).
 */
GP_Context *GP_ReadImage(GP_IO *io, GP_ProgressCallback *callback);

/*
 * Tries to load image accordingly to the file extension.
 *
 * If operation fails NULL is returned and errno is filled.
 */
GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback);

/*
 * Loads image Meta Data (if possible).
 */
int GP_LoadMetaData(const char *src_path, GP_MetaData *data);

/*
 * Simple saving function, the image format is matched by file extension.
 *
 * Retruns zero on success.
 *
 * On failure non-zero is returned.
 *
 * When file type wasn't recognized by extension or if support for requested
 * image format wasn't compiled in non-zero is returned and errno is set to
 * ENOSYS.
 *
 * The resulting errno may also be set to any possible error from fopen(3), open(3),
 * write(3), fwrite(3), seek(3), etc..
 */
int GP_SaveImage(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback);

/*
 * Describes image loader/saver.
 */
typedef struct GP_Loader {
	/*
	 * Reads an image from an IO stream.
	 *
	 * Returns newly allocated context cotaining the loaded image or in
	 * case of failure NULL and errno is set.
	 */
	GP_Context *(*Read)(GP_IO *io, GP_ProgressCallback *callback);

	/*
	 * Loads an image from a file.
	 *
	 * TODO: Remove due to Read
	 */
	GP_Context *(*Load)(const char *src_path, GP_ProgressCallback *callback);

	/*
	 * Save an image.
	 *
	 * Returns zero on succes, non-zero on failure and errno must be set.
	 */
	int (*Save)(const GP_Context *src, const char *dst_path, GP_ProgressCallback *callback);

	/*
	 * The buffer is filled with 32 bytes from an image start, returns 1 if
	 * image signature was found zero otherwise.
	 */
	int (*Match)(const void *buf);

	/*
	 * Short format name.
	 */
	const char *fmt_name;

	/* don't touch */
	struct GP_Loader *next;

	/*
	 * NULL terminated array of file extensions.
	 */
	const char *extensions[];
} GP_Loader;

/*
 * Takes pointer to buffer at least 32 bytes long and returns a pointer to
 * matched loader or NULL.
 */
const GP_Loader *GP_MatchSignature(const void *buf);

/*
 * Tries to match loader by extension. Returns NULL if no loader was found.
 */
const GP_Loader *GP_MatchExtension(const char *path);

void GP_LoaderRegister(GP_Loader *self);

void GP_LoaderUnregister(GP_Loader *self);

/*
 * List loaders into the stdout
 */
void GP_ListLoaders(void);

#endif /* LOADERS_GP_LOADER_H */
