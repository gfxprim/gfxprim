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
#include "loaders/GP_DataStorage.h"

/*
 * Reads an image from a I/O stream.
 *
 * The image format is matched from the file signature (first few bytes of the
 * I/O stream).
 */
GP_Context *GP_ReadImage(GP_IO *io, GP_ProgressCallback *callback);

int GP_ReadImageEx(GP_IO *io, GP_Context **img, GP_DataStorage *meta_data,
                   GP_ProgressCallback *callback);

/*
 * Tries to load image accordingly to the file extension.
 *
 * If operation fails NULL is returned and errno is filled.
 */
GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback);

int GP_LoadImageEx(const char *src_path,
                   GP_Context **img, GP_DataStorage *meta_data,
                   GP_ProgressCallback *callback);

/*
 * Loads image Meta Data (if possible).
 */
int GP_LoadMetaData(const char *src_path, GP_DataStorage *storage);

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
	 * Reads image and/or metadata from an I/O stream.
	 */
	int (*Read)(GP_IO *io, GP_Context **img, GP_DataStorage *storage,
                    GP_ProgressCallback *callback);

	/*
	 * Writes an image into an I/O stream.
	 *
	 * Returns zero on success, non-zero on failure and errno must be set.
	 */
	int (*Write)(const GP_Context *src, GP_IO *io,
	             GP_ProgressCallback *callback);

	/*
	 * GP_PIXEL_UNKNOWN terminated array of formats loader supports for save.
	 *
	 * This is _NOT_ a complete list loaders is able to save, due to automatic
	 * conversions (i.e. RGB888 vs BRG888).
	 */
	const GP_PixelType *save_ptypes;

	/*
	 * The buffer is filled with 32 bytes from an image start, returns 1 if
	 * image signature was found zero otherwise.
	 */
	int (*Match)(const void *buf);

	/*
	 * Short format name.
	 */
	const char *fmt_name;

	/*
	 * NULL terminated array of file extensions.
	 */
	const char *extensions[];
} GP_Loader;

/*
 * Takes pointer to buffer at least 32 bytes long and returns a pointer to
 * matched loader or NULL.
 */
const GP_Loader *GP_LoaderBySignature(const void *buf);

/*
 * Tries to match loader by filename extension. Returns NULL if no loader was
 * found.
 */
const GP_Loader *GP_LoaderByFilename(const char *path);

/*
 * Registers additional loader.
 *
 * Returns zero on success, non-zero if table of loaders was is full.
 */
int GP_LoaderRegister(const GP_Loader *self);

/*
 * Unregisters a loader.
 *
 * All library loaders are registered by default.
 *
 * You can unregister them using this function if you want.
 */
void GP_LoaderUnregister(const GP_Loader *self);

/*
 * Generic LoadImage for a given loader.
 *
 * The function prepares the I/O from file, calls the loader Read() method,
 * closes the I/O and returns the context.
 */
GP_Context *GP_LoaderLoadImage(const GP_Loader *self, const char *src_path,
                               GP_ProgressCallback *callback);

/*
 * Generic ReadImage for a given loader.
 *
 * The function calls the loader Read() method for a given I/O.
 */
GP_Context *GP_LoaderReadImage(const GP_Loader *self, GP_IO *io,
                               GP_ProgressCallback *callback);

/*
 * Generic ReadImageEx for a given loader.
 *
 * The function calls the loader Read() method for a given I/O.
 */
int GP_LoaderReadImageEx(const GP_Loader *self, GP_IO *io,
                         GP_Context **img, GP_DataStorage *data,
                         GP_ProgressCallback *callback);

/*
 * Generic LoadImageEx for a given loader.
 *
 * The function prepares the I/O from file, calls the loader ReadEx() method,
 * closes the I/O and returns the context.
 */
int GP_LoaderLoadImageEx(const GP_Loader *self, const char *src_path,
                         GP_Context **img, GP_DataStorage *data,
                         GP_ProgressCallback *callback);

/*
 * Generic SaveImage for a given loader.
 *
 * The function/ prepares the I/O from file, calls the loader Write() method
 * and closes the I/O.
 */
int GP_LoaderSaveImage(const GP_Loader *self, const GP_Context *src,
                       const char *dst_path, GP_ProgressCallback *callback);

/*
 * List loaders into the stdout
 */
void GP_ListLoaders(void);

#endif /* LOADERS_GP_LOADER_H */
