// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Image loader.

   This is abstraction on the top of the image list and image cache.

  */

#ifndef IMAGE_LOADER_H__
#define IMAGE_LOADER_H__

#include <core/gp_pixmap.h>
#include <core/gp_progress_callback.h>

/*
 * Initialize image loader.
 */
int image_loader_init(const char *args[], unsigned int cache_max_bytes);

/*
 * Fills pointer to current image.
 *
 * Returns zero or, in case of failure, errno.
 *
 * Note that the callback may not be called when the image is cached.
 */
gp_pixmap *image_loader_get_image(gp_progress_cb *callback, int elevate);

/*
 * Retruns current image meta data or NULL there are none.
 */
gp_storage *image_loader_get_meta_data(void);

/*
 * Returns path to current image.
 */
const char *image_loader_img_path(void);

/*
 * Returns image name.
 *
 * Note that in case of image containers this is different from the image path.
 */
const char *image_loader_img_name(void);


enum img_seek_offset {
	IMG_FIRST,
	IMG_LAST,
	IMG_CUR,
	/*
	 * Seeks to the last image in the directory if whence > 0
	 * to the first image in dir otherwise.
	 */
	IMG_DIR,
};

/*
 * Changes position in image list.
 */
void image_loader_seek(enum img_seek_offset offset, int whence);

/*
 * Drops image cache.
 */
void image_loader_drop_cache(void);

/*
 * Free all memory, close all files.
 */
void image_loader_destroy(void);

/*
 * Counts images in the image list.
 */
unsigned int image_loader_count(void);

/*
 * Returns current position.
 */
unsigned int image_loader_pos(void);


/*
 * Returns non-zero if we are in diretory.
 */
int image_loader_is_in_dir(void);

/*
 * Counts number of images in current directory.
 */
unsigned int image_loader_dir_count(void);

/*
 * Returns current position in directory.
 */
unsigned int image_loader_dir_pos(void);

#endif /* IMAGE_LOADER_H__ */
