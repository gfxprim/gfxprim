// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Core include file for loaders API.

  */

#ifndef LOADERS_GP_LOADER_H
#define LOADERS_GP_LOADER_H

#include "core/gp_pixmap.h"
#include <core/gp_progress_callback.h>

#include <loaders/gp_io.h>
#include <loaders/gp_data_storage.h>

/*
 * Reads an image from a I/O stream.
 *
 * The image format is matched from the file signature (first few bytes of the
 * I/O stream).
 */
gp_pixmap *gp_read_image(gp_io *io, gp_progress_cb *callback);

int gp_read_image_ex(gp_io *io, gp_pixmap **img, gp_storage *meta_data,
                     gp_progress_cb *callback);

/*
 * Tries to load image accordingly to the file extension.
 *
 * If operation fails NULL is returned and errno is filled.
 */
gp_pixmap *gp_load_image(const char *src_path, gp_progress_cb *callback);

int gp_load_image_ex(const char *src_path,
                     gp_pixmap **img, gp_storage *meta_data,
                     gp_progress_cb *callback);

/*
 * Loads image Meta Data (if possible).
 */
int gp_load_meta_data(const char *src_path, gp_storage *storage);

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
int gp_save_image(const gp_pixmap *src, const char *dst_path,
                  gp_progress_cb *callback);

typedef struct gp_loader gp_loader;

/*
 * Describes image loader/saver.
 */
struct gp_loader {
	/*
	 * Reads image and/or metadata from an I/O stream.
	 */
	int (*Read)(gp_io *io, gp_pixmap **img, gp_storage *storage,
                    gp_progress_cb *callback);

	/*
	 * Writes an image into an I/O stream.
	 *
	 * Returns zero on success, non-zero on failure and errno must be set.
	 */
	int (*Write)(const gp_pixmap *src, gp_io *io,
	             gp_progress_cb *callback);

	/*
	 * GP_PIXEL_UNKNOWN terminated array of formats loader supports for save.
	 *
	 * This is _NOT_ a complete list loaders is able to save, due to automatic
	 * conversions (i.e. RGB888 vs BRG888).
	 */
	const gp_pixel_type *save_ptypes;

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
};

/*
 * Takes pointer to buffer at least 32 bytes long and returns a pointer to
 * matched loader or NULL.
 */
const gp_loader *gp_loader_by_signature(const void *buf);

/*
 * Tries to match loader by filename extension. Returns NULL if no loader was
 * found.
 */
const gp_loader *gp_loader_by_filename(const char *path);

/*
 * Registers additional loader.
 *
 * Returns zero on success, non-zero if table of loaders was is full.
 */
int gp_loader_register(const gp_loader *self);

/*
 * Unregisters a loader.
 *
 * All library loaders are registered by default.
 *
 * You can unregister them using this function if you want.
 */
void gp_loader_unregister(const gp_loader *self);

/*
 * Generic LoadImage for a given loader.
 *
 * The function prepares the I/O from file, calls the loader Read() method,
 * closes the I/O and returns the pixmap.
 */
gp_pixmap *gp_loader_load_image(const gp_loader *self, const char *src_path,
                                gp_progress_cb *callback);

/*
 * Generic ReadImage for a given loader.
 *
 * The function calls the loader Read() method for a given I/O.
 */
gp_pixmap *gp_loader_read_image(const gp_loader *self, gp_io *io,
                                gp_progress_cb *callback);

/*
 * Generic ReadImageEx for a given loader.
 *
 * The function calls the loader Read() method for a given I/O.
 */
int gp_loader_read_image_ex(const gp_loader *self, gp_io *io,
                            gp_pixmap **img, gp_storage *data,
                            gp_progress_cb *callback);

/*
 * Generic LoadImageEx for a given loader.
 *
 * The function prepares the I/O from file, calls the loader ReadEx() method,
 * closes the I/O and returns the pixmap.
 */
int gp_loader_load_image_ex(const gp_loader *self, const char *src_path,
                            gp_pixmap **img, gp_storage *data,
                            gp_progress_cb *callback);

/*
 * Generic SaveImage for a given loader.
 *
 * The function/ prepares the I/O from file, calls the loader Write() method
 * and closes the I/O.
 */
int gp_loader_save_image(const gp_loader *self, const gp_pixmap *src,
                         const char *dst_path, gp_progress_cb *callback);

/*
 * List loaders into the stdout
 */
void gp_loaders_lists(void);

#endif /* LOADERS_GP_LOADER_H */
