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

#include <core/gp_pixmap.h>
#include <core/gp_debug.h>
#include <filters/GP_ResizeNN.h>
#include <filters/GP_ResizeLinear.h>
#include <filters/GP_ResizeCubic.h>
#include <filters/GP_Resize.h>

static const char *interp_types[] = {
	"Nearest Neighbour",
	"Linear (Int)",
	"Linear with Low Pass (Int)",
	"Cubic (Float)",
	"Cubic (Int)",
};

const char *gp_interpolation_type_name(enum gp_interpolation_type interp_type)
{
	if (interp_type > GP_INTERP_MAX)
		return "Unknown";

	return interp_types[interp_type];
}

static int resize(const gp_pixmap *src, gp_pixmap *dst,
                  gp_interpolation_type type,
                  gp_progress_cb *callback)
{
	switch (type) {
	case GP_INTERP_NN:
		return gp_filter_resize_nn(src, dst, callback);
	case GP_INTERP_LINEAR_INT:
		return gp_filter_resize_linear_int(src, dst, callback);
	case GP_INTERP_LINEAR_LF_INT:
		return gp_filter_resize_linear_lf_int(src, dst, callback);
	case GP_INTERP_CUBIC:
		return gp_filter_resize_cubic(src, dst, callback);
	case GP_INTERP_CUBIC_INT:
		return gp_filter_resize_cubic_int(src, dst, callback);
	}

	GP_WARN("Invalid interpolation type %u", (unsigned int)type);

	errno = EINVAL;
	return 1;
}

int gp_filter_resize(const gp_pixmap *src, gp_pixmap *dst,
                     gp_interpolation_type type,
                     gp_progress_cb *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize(src, dst, type, callback);
}

gp_pixmap *gp_filter_resize_alloc(const gp_pixmap *src,
                                  gp_size w, gp_size h,
                                  gp_interpolation_type type,
                                  gp_progress_cb *callback)
{
	gp_pixmap *res;

	if (!w && !h) {
		GP_WARN("Invalid result size 0x0!");
		errno = EINVAL;
		return NULL;
	}

	if (!w)
		w = (h * src->w + src->h/2) / src->h;

	if (!h)
		h = (w * src->h + src->w/2) / src->w;

	res = gp_pixmap_alloc(w, h, src->pixel_type);
	if (!res)
		return NULL;

	if (resize(src, res, type, callback)) {
		gp_pixmap_free(res);
		return NULL;
	}

	return res;
}
