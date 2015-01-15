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

#include "core/GP_Context.h"
#include "core/GP_ProgressCallback.h"

#include "loaders/GP_DataStorage.h"

struct GP_Container;

enum GP_ContainerWhence {
	GP_CONT_FIRST,
	GP_CONT_LAST,
	GP_CONT_CUR,
};

struct GP_ContainerOps {
	/*
	 * Loads next image from container, use the inline function defined
	 * below.
	 */
	GP_Context *(*LoadNext)(struct GP_Container *self,
	                        GP_ProgressCallback *callback);

	/*
	 * Just loads current image, does not advance to the next image.
	 */
	int (*LoadEx)(struct GP_Container *self, GP_Context **img,
	              GP_DataStorage *storage, GP_ProgressCallback *callback);

	/*
	 * Close callback, use the inline function defined below.
	 */
	void (*Close)(struct GP_Container *self);

	/*
	 * Seeks to the offset from whence.
	 *
	 * Returns 0 on success, errno on failure.
	 */
	int (*Seek)(struct GP_Container *self, int offset,
	            enum GP_ContainerWhence whence);

	/*
	 * Container type name.
	 */
	const char *type;
};

typedef struct GP_Container {
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
	const struct GP_ContainerOps *ops;

	char priv[];
} GP_Container;

#define GP_CONTAINER_PRIV(c) ((void*)(c)->priv)

/*
 * Behaves just like GP_LoadImage, but takes pointer to opened container instead.
 */
static inline GP_Context *GP_ContainerLoadNext(GP_Container *self,
                                               GP_ProgressCallback *callback)
{
	return self->ops->LoadNext(self, callback);
}

/*
 * Just loads current image, does not advance to the next one.
 */
int GP_ContainerLoadEx(GP_Container *self, GP_Context **img,
                       GP_DataStorage *storage, GP_ProgressCallback *callback);

static inline GP_Context *GP_ContainerLoad(GP_Container *self,
                                           GP_ProgressCallback *callback)
{
	GP_Context *ret = NULL;

	GP_ContainerLoadEx(self, &ret, NULL, callback);

	return ret;
}

int GP_ContainerSeek(GP_Container *self, int offset,
                     enum GP_ContainerWhence whence);

static inline void GP_ContainerClose(GP_Container *self)
{
	self->ops->Close(self);
}

#endif /* LOADERS_GP_CONTAINER_H */
