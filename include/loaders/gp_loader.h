// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @brief Core include file for loaders API.
  * @file gp_loader.h
  */

#ifndef LOADERS_GP_LOADER_H
#define LOADERS_GP_LOADER_H

#include <core/gp_pixmap.h>
#include <core/gp_progress_callback.h>
#include <loaders/gp_io.h>
#include <loaders/gp_data_storage.h>

/**
 * @brief An image info and meta data.
 *
 * Filled in by the image loaders if passed.
 *
 * The dimensions and the pixel type are filled in as soon as the image header
 * is parsed — even when the bitmap itself cannot be decoded.
 */
typedef struct gp_image_info {
	/** @brief An image width in pixels. */
	gp_size w;

	/** @brief An image height in pixels. */
	gp_size h;

	/**
	 * @brief A pixel type the decoded pixmap would have.
	 *
	 * GP_PIXEL_UNKNOWN when the image cannot be decoded or the pixel type
	 * mapping is not known yet.
	 */
	gp_pixel_type ptype;

	/**
	 * @brief An optional metadata storage, may be NULL.
	 *
	 * If needed the pointer should be initialized with
	 * gp_storage_create().
	 */
	gp_storage *meta_data;
} gp_image_info;

/**
 * @brief Returns pointer meta data in image info if both exist.
 *
 * @param image_info An image info.
 * @return A pointer to a storage if both image info and meta data storage exists.
 */
static inline gp_storage *gp_image_info_meta_data(gp_image_info *image_info)
{
	return image_info ? image_info->meta_data : NULL;
}

/**
 * @brief Clears image info.
 *
 * Sets the w and h to 0 and ptype to GP_PIXEL_UNKNOWN.
 *
 * This is called at the start of an image loading to clear the image info.
 *
 * @param image_info An image info to be cleared.
 */
static inline void gp_image_info_clear(gp_image_info *image_info)
{
	if (!image_info)
		return;

	image_info->w = 0;
	image_info->h = 0;
	image_info->ptype = GP_PIXEL_UNKNOWN;
}

/**
 * @brief Sets the image info and basic meta data.
 *
 * This is called by image loaders once image header was read.
 *
 * @param image_info An image info to be filled in.
 * @param w Image width after loading.
 * @param h Image height after loading.
 * @param ptype Image pixel type after loading.
 */
static inline void gp_image_info_fill(gp_image_info *image_info,
                                      gp_size w, gp_size h, gp_pixel_type ptype)
{
	if (!image_info)
		return;

	image_info->w = w;
	image_info->h = h;
	image_info->ptype = ptype;

	if (!image_info->meta_data)
		return;

	gp_storage_add_int(image_info->meta_data, NULL, "Width", w);
	gp_storage_add_int(image_info->meta_data, NULL, "Height", h);
}

/**
 * @brief Reads an image from a I/O stream.
 *
 * The image format is matched from the file signature i.e. first few bytes of the
 * I/O stream.
 *
 * @param io An I/O.
 * @param callback A progress callback.
 * @return Newly allocated and initialized image or in a case of a failure NULL
 *         and errno is set. The resulting errno may also be set to any
 *         possible error from open(3), read(3), seek(3), etc.
 */
gp_pixmap *gp_read_image(gp_io *io, gp_progress_cb *callback);

/**
 * @brief Reads an image from a I/O stream.
 *
 * The image format is matched from the file signature i.e. first few bytes of the
 * I/O stream.
 *
 * @param io An I/O.
 * @param img A pointer to store the loaded image to.
 * @param image_info An image info to be filled in may be NULL if not needed.
 * @param callback A progress callback.
 * @return Zero on success, non-zero on a failure and errno is set. The resulting
 *         errno may also be set to any possible error from open(3), read(3),
 *         seek(3), etc.
 */
int gp_read_image_ex(gp_io *io, gp_pixmap **img, gp_image_info *image_info,
                     gp_progress_cb *callback);

/**
 * @brief Loads an image file.
 *
 * Tries to load image accordingly to the file extension, but falls back to
 * signature detection if loader for the file extension fails.
 *
 * @param src_path A path to an image file.
 * @param callback A progress callback.
 * @return Newly allocated and initialized image or in a case of a failure NULL
 *         and errno is set. The resulting errno may also be set to any
 *         possible error from open(3), read(3), seek(3), etc.
 */
gp_pixmap *gp_load_image(const char *src_path, gp_progress_cb *callback);

/**
 * @brief Loads an image file.
 *
 * Tries to load image accordingly to the file extension, but falls back to
 * signature detection if loader for the file extension fails.
 *
 * @param src_path A path to an image file.
 * @param img A pointer to store the loaded image to.
 * @param image_info Image info to be filled in, may be NULL if not needed.
 * @param callback A progress callback.
 * @return Zero on success, non-zero on a failure and errno is set. The
 *         resulting errno may also be set to any possible error from open(3),
 *         read(3), seek(3), etc.
 */
int gp_load_image_ex(const char *src_path,
                     gp_pixmap **img, gp_image_info *image_info,
                     gp_progress_cb *callback);

/**
 * @brief Loads image meta data.
 *
 * Loads only image metadata, e.g. exif. No bitmap data are loaded.
 *
 * @param src_path A path to a image file.
 * @param meta_data A data storage for the metadata.
 * @return Zero on success, non-zero otherwise.
 */
int gp_load_meta_data(const char *src_path, gp_storage *meta_data);

/**
 * @brief Simple saving function, the image format is guessed by the file
 *        extension.
 *
 * When file type wasn't recognized by extension or if support for requested
 * image format wasn't compiled in non-zero is returned and errno is set to
 * ENOSYS.
 *
 * @warning Image loader can save only pixmaps with a compatible pixel types.
 *
 * @param src A pixmap to be saved.
 * @param dst_path A path to a file to save the image into.
 * @return Zero on success, non-zero otherwise and errno is set. The resulting
 *         errno may also be set to any possible error from open(3), write(3),
 *         seek(3), etc.
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
	int (*read)(gp_io *io, gp_pixmap **img, gp_image_info *image_info,
                    gp_progress_cb *callback);

	/*
	 * Writes an image into an I/O stream.
	 *
	 * Returns zero on success, non-zero on failure and errno must be set.
	 */
	int (*write)(const gp_pixmap *src, gp_io *io,
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
	int (*match)(const void *buf);

	/*
	 * Short format name.
	 */
	const char *fmt_name;

	/*
	 * NULL terminated array of file extensions.
	 */
	const char *extensions[];
};

/**
 * @brief Looks up a loader by a file signature.
 *
 * @param buf First 32 bytes of the image.
 * @return A pointer to loader or NULL if not found.
 */
const gp_loader *gp_loader_by_signature(const void *buf);

/**
 * @brief Looks up a loader by a file extension.
 *
 * @param path A path to an image file.
 * @return A pointer to loader or NULL if not found.
 */
const gp_loader *gp_loader_by_filename(const char *path);

/**
 * @brief Registers a loader.
 *
 * @param self A loader to be registered.
 * @return Zero on success, non-zero if table of loaders was is full.
 */
int gp_loader_register(const gp_loader *self);

/**
 * @brief Unregisters a loader.
 *
 * All library loaders are registered by default.
 *
 * You can unregister them using this function.
 *
 * @param self A loader to be unregistered.
 */
void gp_loader_unregister(const gp_loader *self);

/**
 * @brief Loads image for a given loader.
 *
 * @param self A loader.
 * @param src_path A path to an image file.
 * @param callback A progress callback.
 * @return Newly allocated and initialized image or in a case of a failure NULL
 *         and errno is set. The resulting errno may also be set to any
 *         possible error from open(3), read(3), seek(3), etc.
 */
gp_pixmap *gp_loader_load_image(const gp_loader *self, const char *src_path,
                                gp_progress_cb *callback);

/**
 * @brief Loads image for a given loader.
 *
 * @param src_path A path to an image file.
 * @param img A pointer to store the loaded image to.
 * @param image_info Image info to be filled in, may be NULL if not needed.
 * @param callback A progress callback.
 * @return Zero on success, non-zero on a failure and errno is set. The
 *         resulting errno may also be set to any possible error from open(3),
 *         read(3), seek(3), etc.
 */
int gp_loader_load_image_ex(const gp_loader *self, const char *src_path,
                            gp_pixmap **img, gp_image_info *image_info,
                            gp_progress_cb *callback);

/**
 * @brief Reads image for a given loader.
 *
 * @param self A loder.
 * @param io An I/O.
 * @param callback A progress callback.
 * @return Newly allocated and initialized image or in a case of a failure NULL
 *         and errno is set. The resulting errno may also be set to any
 *         possible error from open(3), read(3), seek(3), etc.
 */
gp_pixmap *gp_loader_read_image(const gp_loader *self, gp_io *io,
                                gp_progress_cb *callback);

/**
 * @brief Reads image for a given loader.
 *
 * @param io An I/O.
 * @param img A pointer to store the loaded image to.
 * @param image_info Image info to be filled in, may be NULL if not needed.
 * @param callback A progress callback.
 * @return Zero on success, non-zero on failure and errno is set. The resulting
 *         errno may also be set to any possible error from open(3), read(3),
 *         seek(3), etc.
 */
int gp_loader_read_image_ex(const gp_loader *self, gp_io *io,
                            gp_pixmap **img, gp_image_info *image_info,
                            gp_progress_cb *callback);

/**
 * @brief Saves image for a given loader.
 *
 * @warning Image loader can save only pixmaps with a compatible pixel types.
 *
 * @param self A loder.
 * @param src A pixmap to be saved.
 * @param dst_path A path to a file to save the image into.
 * @return Zero on success, non-zero otherwise and errno is set. The resulting
 *         errno may also be set to any possible error from open(3), write(3),
 *         seek(3), etc.
 */
int gp_loader_save_image(const gp_loader *self, const gp_pixmap *src,
                         const char *dst_path, gp_progress_cb *callback);

/**
 * @brief Lists all registered loaders into the stdout.
 */
void gp_loaders_list(void);

#endif /* LOADERS_GP_LOADER_H */
