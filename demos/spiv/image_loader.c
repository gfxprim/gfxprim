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

#include <errno.h>

#include <core/GP_Context.h>
#include <core/GP_Debug.h>

#include <loaders/GP_Loaders.h>

#include "cpu_timer.h"
#include "image_cache.h"
#include "image_list.h"
#include "image_loader.h"

static struct image_cache *img_cache;
static struct image_list *img_list;
static GP_Context *cur_img;
static GP_Container *cur_cont;

int image_loader_init(const char *args[], unsigned int cache_max_bytes)
{
	img_cache = image_cache_create(cache_max_bytes);

	if (!img_cache) {
		GP_WARN("Failed to initialize image cache (size=%u)",
		        cache_max_bytes);
	}

	img_list = image_list_create(args);

	if (!img_list) {
		GP_FATAL("Failed to initialize image list");
		return 1;
	}

	return 0;
}

GP_Context *image_loader_get_image(GP_ProgressCallback *callback, int elevate)
{
	struct cpu_timer timer;
	const char *path;
	GP_Context *img;
	int err;

	if (cur_img)
		return cur_img;

	if (cur_cont) {
		cur_img = GP_ContainerLoad(cur_cont, callback);
		return cur_img;
	}

	path = image_list_img_path(img_list);

	cur_img = image_cache_get(img_cache, elevate, path);

	if (cur_img)
		return cur_img;

	cpu_timer_start(&timer, "Loading");

	img = GP_LoadImage(path, callback);

	if (!img) {
		err = errno;

		/*
		 * Try containers, ZIP for now, more to come
		 *
		 * TODO: How to cache container content?
		 */
		cur_cont = GP_OpenZip(path);

		if (cur_cont) {
			img = GP_ContainerLoad(cur_cont, callback);

			if (img) {
				cur_img = img;
				return img;
			}

			GP_ContainerClose(cur_cont);
			cur_cont = NULL;
		}

		errno = err;
		return NULL;
	}

	image_cache_put(img_cache, img, path);

	cpu_timer_stop(&timer);

	return img;
}

const char *image_loader_img_path(void)
{
	//TODO: Make this more elegant
	static char path[512];

	if (cur_cont) {
		snprintf(path, sizeof(path), "%s:%u",
		         image_list_img_path(img_list), cur_cont->cur_img);
		return path;
	}

	return image_list_img_path(img_list);
}

const char *image_loader_img_name(void)
{
	return image_list_img_path(img_list);
}

static void drop_cur_img(void)
{
	const char *path;

	path = image_list_img_path(img_list);

	/*
	 * Currently loaded image is too big to be cached -> free it.
	 */
	if (!image_cache_get(img_cache, 0, path))
		GP_ContextFree(cur_img);

	cur_img = NULL;
}

void image_loader_seek(enum img_seek_offset offset, int whence)
{
	drop_cur_img();

	if (cur_cont) {
		switch (offset) {
		case IMG_FIRST:
		case IMG_LAST:
		//TODO  do something better for IMG_DIR
		case IMG_DIR:
			GP_ContainerClose(cur_cont);
			cur_cont = NULL;
			goto list_seek;
		case IMG_CUR:
		break;
		}
		/*
		 * TODO: We should be able to count how much
		 *       we get out of the container and seek
		 *       N images in the list
		 *
		 *       What about wrapping around?
		 */
		if (GP_ContainerSeek(cur_cont, whence, GP_CONT_CUR)) {
			GP_ContainerClose(cur_cont);
			cur_cont = NULL;
			goto list_seek;
		}

		return;
	}

list_seek:

	switch (offset) {
	case IMG_FIRST:
		image_list_first(img_list);
	break;
	case IMG_LAST:
		image_list_last(img_list);
	break;
	case IMG_CUR:
	break;
	case IMG_DIR:
		image_list_dir_move(img_list, whence);
		return;
	}

	if (!whence)
		return;

	image_list_move(img_list, whence);
}

unsigned int image_loader_count(void)
{
	return image_list_count(img_list);
}

unsigned int image_loader_pos(void)
{
	return image_list_pos(img_list);
}

int image_loader_is_in_dir(void)
{
	return image_list_dir_count(img_list) != 0;
}

unsigned int image_loader_dir_count(void)
{
	return image_list_dir_count(img_list);
}

unsigned int image_loader_dir_pos(void)
{
	return image_list_dir_pos(img_list);
}

void image_loader_drop_cache(void)
{
	image_cache_drop(img_cache);
}

void image_loader_destroy(void)
{
	GP_DEBUG(1, "Destroying loader");
	drop_cur_img();
	GP_DEBUG(1, "Destroying cache");
	image_cache_destroy(img_cache);
	GP_DEBUG(1, "Destroying image list");
	image_list_destroy(img_list);
}
