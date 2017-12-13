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

   Container is an abstraction for file formats that can include several images
   in one file. For example gif, tiff, apng, cbr, cbz, etc.

  */

#ifndef LOADERS_GP_CONTAINER_H
#define LOADERS_GP_CONTAINER_H

#include "core/GP_Pixmap.h"
#include "core/GP_ProgressCallback.h"

#include "loaders/GP_DataStorage.h"

typedef struct gp_container gp_container;

enum gp_container_whence {
	GP_CONT_FIRST,
	GP_CONT_LAST,
	GP_CONT_CUR,
};

struct gp_container_ops {
	/*
	 * Loads next image from container, use the inline function defined
	 * below.
	 */
	gp_pixmap *(*load_next)(gp_container *self, gp_progress_cb *callback);

	/*
	 * Just loads current image, does not advance to the next image.
	 */
	int (*load_ex)(gp_container *self, gp_pixmap **img,
	               gp_storage *storage, gp_progress_cb *callback);

	/*
	 * Close callback, use the inline function defined below.
	 */
	void (*close)(gp_container *self);

	/*
	 * Seeks to the offset from whence.
	 *
	 * Returns 0 on success, errno on failure.
	 */
	int (*seek)(gp_container *self, int offset,
	            enum gp_container_whence whence);

	/*
	 * Container type name.
	 */
	const char *type;
};

typedef struct gp_container {
	/*
	 * Image counter. This is set to number of images, or to -1 if number
	 * of images in container is not known prior to parsing the whole
	 * file.
	 */
	unsigned int img_count;

	/*
	 * Current image counter, do not change from application.
	 */
	unsigned int cur_img;

	/*
	 * Contains container callbacks
	 */
	const struct gp_container_ops *ops;

	char priv[];
} gp_container;

#define GP_CONTAINER_PRIV(c) ((void*)(c)->priv)

/*
 * Behaves just like gp_load_image, but takes pointer to opened container instead.
 */
static inline gp_pixmap *gp_container_load_next(gp_container *self,
                                                gp_progress_cb *callback)
{
	return self->ops->load_next(self, callback);
}

/*
 * Just loads current image, does not advance to the next one.
 */
int gp_container_load_ex(gp_container *self, gp_pixmap **img,
                         gp_storage *storage, gp_progress_cb *callback);

static inline gp_pixmap *gp_container_load(gp_container *self,
                                           gp_progress_cb *callback)
{
	gp_pixmap *ret = NULL;

	gp_container_load_ex(self, &ret, NULL, callback);

	return ret;
}

int gp_container_seek(gp_container *self, int offset,
                      enum gp_container_whence whence);

static inline void gp_container_close(gp_container *self)
{
	self->ops->close(self);
}

#endif /* LOADERS_GP_CONTAINER_H */
