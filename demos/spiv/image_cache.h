// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Image loader chache.

  */

#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

struct image_cache;

/*
 * Returns size of the ram in kbytes.
 */
size_t image_cache_get_ram_size(void);

/*
 * Creates an image cache with maximal memory size of max_size_kbytes.
 */
struct image_cache *image_cache_create(unsigned int max_size_kbytes);

/*
 * Returns cached image, or NULL.
 *
 * If elevate set and image is found, the image is elevated to the top so
 * it has lesser chance of being freed.
 */
int image_cache_get(struct image_cache *self, gp_pixmap **img,
		    gp_storage **meta_data, int elevate,
                    const char *key);

gp_pixmap *image_cache_get2(struct image_cache *self, int elevate,
                             const char *fmt, ...)
                             __attribute__ ((format (printf, 3, 4)));
/*
 * Puts an image into a cache.
 */
int image_cache_put(struct image_cache *self, gp_pixmap *img,
                    gp_storage *meta_data, const char *key);

int image_cache_put2(struct image_cache *self, gp_pixmap *img,
                     gp_storage *meta_data, const char *fmt, ...)
                     __attribute__ ((format (printf, 4, 5)));

/*
 * Drop all image in cache.
 */
void image_cache_drop(struct image_cache *self);

/*
 * Destroys image cache and all it's images.
 */
void image_cache_destroy(struct image_cache *self);

/*
 * Print the image cache content.
 */
void image_cache_print(struct image_cache *self);

#endif /* IMAGE_CACHE_H */
