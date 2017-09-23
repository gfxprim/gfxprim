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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Image loader.

   This is abstraction on the top of the image list and image cache.

  */

#ifndef __IMAGE_LOADER_H__
#define __IMAGE_LOADER_H__

#include <core/GP_Pixmap.h>
#include <core/GP_ProgressCallback.h>

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
GP_Pixmap *image_loader_get_image(GP_ProgressCallback *callback, int elevate);

/*
 * Retruns current image meta data or NULL there are none.
 */
GP_DataStorage *image_loader_get_meta_data(void);

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

#endif /* __IMAGE_LOADER_H__ */
