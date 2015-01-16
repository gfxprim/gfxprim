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

#include <stdarg.h>
#include <string.h>
#include <GP.h>
#include "image_cache.h"

struct image {
	GP_Context *ctx;
	GP_DataStorage *meta_data;

	struct image *prev;
	struct image *next;

	/* number of elevated get calls */
	unsigned int elevated;

	/* this identifies an image */
	char path[];
};

struct image_cache {
	unsigned int max_size;
	unsigned int cur_size;

	struct image *root;
	struct image *end;
};

size_t image_cache_get_ram_size(void)
{
	FILE *f;
	size_t ret;

	f = fopen("/proc/meminfo", "r");

	if (f == NULL) {
		GP_WARN("Failed to read /proc/meminfo");
		return 0;
	}

	if (fscanf(f, "MemTotal: %zu", &ret) != 1) {
		fclose(f);
		GP_WARN("Failed to read /proc/meminfo");
		return 0;
	}

	fclose(f);

	return ret;
}

/*
 * Reports correct image record size.
 */
static size_t image_size2(GP_Context *ctx, GP_DataStorage *meta_data,
                          const char *path)
{
	size_t meta_data_size = 0;
	size_t context_size = ctx->bytes_per_row * ctx->h + sizeof(GP_Context);

	//TODO! 4096 is a size of single block, data storage may have more blocks
	if (meta_data)
		meta_data_size = 4096;

	return meta_data_size + context_size +
	       sizeof(struct image) + strlen(path) + 1;
}

static size_t image_size(struct image *img)
{
	return image_size2(img->ctx, NULL, img->path);
}

struct image_cache *image_cache_create(unsigned int max_size_kbytes)
{
	struct image_cache *self;

	self = malloc(sizeof(struct image_cache));

	if (self == NULL)
		return NULL;

	self->max_size = max_size_kbytes * 1024;
	self->cur_size = sizeof(struct image_cache);

	self->root = NULL;
	self->end = NULL;

	GP_DEBUG(1, "Created image cache max size %ukB", max_size_kbytes);

	return self;
}

static void remove_img(struct image_cache *self, struct image *img, size_t size)
{
	if (img == self->end)
		self->end = img->prev;

	if (img->prev)
		img->prev->next = img->next;

	if (img->next)
		img->next->prev = img->prev;

	if (img == self->root)
		self->root = img->next;

	self->cur_size -= size;
}

static void remove_img_free(struct image_cache *self,
                            struct image *img, size_t size)
{
	GP_DEBUG(2, "Freeing image '%s' size %zu", img->path, size);

	remove_img(self, img, size);
	GP_ContextFree(img->ctx);
	GP_DataStorageDestroy(img->meta_data);
	free(img);
}

/*
 * Adds image to the start of the double linked list
 */
static void add_img(struct image_cache *self, struct image *img, size_t size)
{
	img->next = self->root;

	if (img->next)
		img->next->prev = img;

	img->prev = NULL;

	self->root = img;
	self->cur_size += size;

	if (self->end == NULL)
		self->end = img;
}

int image_cache_get(struct image_cache *self, GP_Context **img,
		    GP_DataStorage **meta_data, int elevate, const char *key)
{
	struct image *i;

	if (self == NULL)
		return 1;

	GP_DEBUG(2, "Looking for image '%s'", key);

	for (i = self->root; i != NULL; i = i->next)
		if (!strcmp(key, i->path))
			break;

	if (i == NULL)
		return 1;

	/* Push the image to the root of the list */
	if (elevate) {
		size_t size = image_size(i);

		GP_DEBUG(2, "Refreshing image '%s'", key);

		remove_img(self, i, size);
		add_img(self, i, size);

		i->elevated++;
	}

	if (img)
		*img = i->ctx;

	if (meta_data)
		*meta_data = i->meta_data;

	return 0;
}

GP_Context *image_cache_get2(struct image_cache *self, int elevate,
                             const char *fmt, ...)
{
	va_list va;
	size_t len;
	char buf[512];
	char *key = buf;
	struct image *i;

	if (self == NULL)
		return NULL;

	va_start(va, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (len >= sizeof(buf)) {
		key = malloc(len + 1);
		if (!key) {
			GP_WARN("Malloc failed :(");
			return NULL;
		}

		va_start(va, fmt);
		vsprintf(key, fmt, va);
		va_end(va);
	}

	GP_DEBUG(2, "Looking for image '%s'", key);

	for (i = self->root; i != NULL; i = i->next)
		if (!strcmp(key, i->path))
			break;

	/* Push the image to the root of the list */
	if (i && elevate) {
		size_t size = image_size(i);

		GP_DEBUG(2, "Refreshing image '%s'", key);

		remove_img(self, i, size);
		add_img(self, i, size);

		i->elevated++;
	}

	if (len >= sizeof(buf))
		free(key);

	return i ? i->ctx : NULL;
}

void image_cache_print(struct image_cache *self)
{
	struct image *i;

	if (self == NULL) {
		printf("Image cache disabled\n");
		return;
	}

	printf("Image cache size %u used %u\n", self->max_size, self->cur_size);

	for (i = self->root; i != NULL; i = i->next)
		printf(" size=%10zu elevated=%u key='%s'\n",
		       image_size(i), i->elevated, i->path);
}

static int assert_size(struct image_cache *self, size_t size)
{
	if (self->cur_size + size < self->max_size)
		return 0;

	while (self->cur_size + size > self->max_size) {

		if (self->end == NULL) {
			GP_WARN("Cache too small for image size %zu", size);
			return 1;
		}

		remove_img_free(self, self->end, image_size(self->end));
	}

	return 0;
}

int image_cache_put(struct image_cache *self, GP_Context *ctx,
                    GP_DataStorage *meta_data, const char *key)
{
	size_t size;

	if (self == NULL)
		return 1;

	size = image_size2(ctx, meta_data, key);

	/*
	 * We try to create room for the image. If this fails we add the image
	 * anyway because we need to store it while we are showing it (and it
	 * will be removed from cache by next image for sure).
	 */
	assert_size(self, size);

	struct image *img = malloc(sizeof(struct image) + strlen(key) + 1);

	if (img == NULL) {
		GP_WARN("Malloc failed :(");
		return 1;
	}

	img->ctx = ctx;
	img->meta_data = meta_data;
	img->elevated = 0;
	strcpy(img->path, key);

	GP_DEBUG(2, "Adding image '%s' size %zu", img->path, size);

	add_img(self, img, size);

	return 0;
}

int image_cache_put2(struct image_cache *self, GP_Context *ctx,
                     GP_DataStorage *meta_data, const char *fmt, ...)
{
	size_t size, len;
	va_list va;

	if (self == NULL)
		return 1;

	va_start(va, fmt);
	len = vsnprintf(NULL, 0, fmt, va);
	va_end(va);

	//TODO: FIX THIS
	size = image_size2(ctx, meta_data, "") + len + 1;

	/*
	 * We try to create room for the image. If this fails we add the image
	 * anyway because we need to store it while we are showing it (and it
	 * will be removed from cache by next image for sure).
	 */
	assert_size(self, size);

	struct image *img = malloc(sizeof(struct image) + len + 1);

	if (img == NULL) {
		GP_WARN("Malloc failed :(");
		return 1;
	}

	img->ctx = ctx;
	img->meta_data = meta_data;
	img->elevated = 0;

	va_start(va, fmt);
	vsprintf(img->path, fmt, va);
	va_end(va);

	GP_DEBUG(2, "Adding image '%s' size %zu",
	         img->path, size);

	add_img(self, img, size);

	return 0;
}

void image_cache_drop(struct image_cache *self)
{
	if (self == NULL)
		return;

	GP_DEBUG(1, "Dropping images in cache");

	while (self->end != NULL)
		remove_img_free(self, self->end, 0);

	self->cur_size = sizeof(struct image_cache);
}

void image_cache_destroy(struct image_cache *self)
{
	if (self == NULL)
		return;

	GP_DEBUG(1, "Destroying image cache");

	while (self->end != NULL)
		remove_img_free(self, self->end, 0);

	free(self);
}
