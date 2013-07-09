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

struct GP_Container;

struct GP_ContainerOps {
	/*
	 * Loads next image from container, use the inline function defined
	 * bellow.
	 */
	GP_Context *(*LoadNext)(struct GP_Container *self,
	                        GP_ProgressCallback *callback);

	/*
	 * Close callback, use the inline function defined bellow.
	 */
	void (*Close)(struct GP_Container *self);

	//TODO: Seek
};

typedef struct GP_Container {
	/*
	 * Image counter. This is set to number of images, or to -1 if number
	 * of images in container is not known prior to parsing the whole
	 * file.
	 */
	int img_count;
	/*
	 * Current image counter, do not change from application.
	 */
	int cur_img;

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

static inline void GP_ContainerClose(GP_Container *self)
{
	self->ops->Close(self);
}

#endif /* LOADERS_GP_CONTAINER_H */
