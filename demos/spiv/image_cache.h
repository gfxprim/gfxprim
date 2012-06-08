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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Image loader chache.

  */

#ifndef __IMAGE_CACHE_H__
#define __IMAGE_CACHE_H__

struct image_cache;

/*
 * Creates an image cache with maximal memory size.
 *
 * When memory size is set to zero it's set to 10% of total machine memory
 * (as reported by /proc/meminfo).
 */
struct image_cache *image_cache_create(unsigned int max_size);

/*
 * Returns cached image, or NULL.
 * 
 * If elevate set and image is found, the image is elevated to the top so
 * it has lesser chance of being freed.
 */
GP_Context *image_cache_get(struct image_cache *self, const char *path,
                            long cookie1, long cookie2, int elevate);

/*
 * Puts an image into a cache.
 */
int image_cache_put(struct image_cache *self, GP_Context *img,
                    const char *path, long cookie1, long cookie2); 

/*
 * Destroys image cache and all it's images.
 */
void image_cache_destroy(struct image_cache *self);

/*
 * Print the image cache content.
 */
void image_cache_print(struct image_cache *self);

#endif /* __IMAGE_CACHE_H__ */
