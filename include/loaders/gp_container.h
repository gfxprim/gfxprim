// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Container is an abstraction for file formats that can include several images
   in one file. For example gif, tiff, apng, cbr, cbz, etc.

  */

#ifndef LOADERS_GP_CONTAINER_H
#define LOADERS_GP_CONTAINER_H

#include <core/gp_types.h>
#include <core/gp_progress_callback.h>
#include <utils/gp_seek.h>
#include <loaders/gp_types.h>

#include <loaders/gp_data_storage.h>

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
	int (*seek)(gp_container *self, ssize_t offset,
	            enum gp_seek_whence whence);


	int (*match)(const void *buf);

	/*
	 * Initializes container.
	 */
	gp_container *(*init)(gp_io *io);

	/* Short format name */
	const char *fmt_name;

	/* NULL terminated list of file extensions */
	const char *const extensions[];
};

struct gp_container {
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
};

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

int gp_container_seek(gp_container *self, ssize_t offset,
                      enum gp_seek_whence whence);

static inline void gp_container_close(gp_container *self)
{
	self->ops->close(self);
}

gp_container *gp_container_open(const char *path);

#endif /* LOADERS_GP_CONTAINER_H */
