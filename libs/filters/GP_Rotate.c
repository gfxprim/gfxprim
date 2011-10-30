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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_Core.h"
#include "core/GP_FnPerBpp.h"

#include "GP_Rotate.h"

#include <string.h>

void GP_FilterMirrorH_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	uint32_t bpr = src->bytes_per_row;
	uint8_t  buf[bpr];
	uint32_t y;

	GP_DEBUG(1, "Mirroring image horizontally %ux%u", src->w, src->h);

	#warning FIXME: non byte aligned pixels

	for (y = 0; y < src->h/2; y++) {
		uint8_t *l1 = dst->pixels + bpr * y;
		uint8_t *l2 = dst->pixels + bpr * (src->h - y - 1);

		memcpy(buf, l1, bpr);
		memcpy(l1, l2, bpr);
		memcpy(l2, buf, bpr);
		
		if (callback != NULL && y % 100 == 0)
			GP_ProgressCallbackReport(callback, 200.00 * y / src->h);
	}

	GP_ProgressCallbackDone(callback);
}

GP_Context *GP_FilterMirrorH(const GP_Context *src, GP_Context *dst,
                             GP_ProgressCallback *callback)
{
	if (dst == NULL) {
		dst = GP_ContextCopy(src, 0);
		
		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
		          "Destination is not big enough");
	}

	GP_FilterMirrorH_Raw(src, dst, callback);

	return dst;
}


GP_Context *GP_FilterMirrorV(const GP_Context *src, GP_Context *dst,
                             GP_ProgressCallback *callback)
{
	if (dst == NULL) {
		dst = GP_ContextCopy(src, 0);
		
		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
		          "Destination is not big enough");
	}
	
	GP_FilterMirrorV_Raw(src, dst, callback);

	return dst;
}

void GP_FilterRotate180_Raw(const GP_Context *src, GP_Context *dst,
                            GP_ProgressCallback *callback)
{
	#warning FIXME: Callbacks, faster algorighm?

	GP_FilterMirrorV_Raw(src, dst, NULL);
	GP_FilterMirrorH_Raw(dst, dst, callback);
}

GP_Context *GP_FilterRotate90(const GP_Context *src, GP_Context *dst,
                              GP_ProgressCallback *callback)
{
	if (dst == NULL) {
		dst = GP_ContextAlloc(src->h, src->w, src->pixel_type);
		
		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
		          "Destination is not big enough");
	}

	GP_FilterRotate90_Raw(src, dst, callback);

	return dst;
}

GP_Context *GP_FilterRotate180(const GP_Context *src, GP_Context *dst,
                               GP_ProgressCallback *callback)
{
	if (dst == NULL) {
		dst = GP_ContextCopy(src, 0);
		
		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
		          "Destination is not big enough");
	}

	GP_FilterRotate180_Raw(src, dst, callback);

	return dst;
}

GP_Context *GP_FilterRotate270(const GP_Context *src, GP_Context *dst,
                               GP_ProgressCallback *callback)
{
	if (dst == NULL) {
		dst = GP_ContextAlloc(src->h, src->w, src->pixel_type);
		
		if (dst == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
		          "Destination is not big enough");
	}

	GP_FilterRotate270_Raw(src, dst, callback);

	return dst;
}

GP_Context *GP_FilterSymmetry(const GP_Context *src, GP_Context *dst,
                              GP_FilterSymmetries symmetry,
			      GP_ProgressCallback *callback)
{
	switch (symmetry) {
	case GP_ROTATE_90:
		return GP_FilterRotate90(src, dst, callback);
	case GP_ROTATE_180:
		return GP_FilterRotate180(src, dst, callback);
	case GP_ROTATE_270:
		return GP_FilterRotate270(src, dst, callback);
	case GP_MIRROR_H:
		return GP_FilterMirrorH(src, dst, callback);
	case GP_MIRROR_V:
		return GP_FilterMirrorV(src, dst, callback);
	}

	return NULL;
}
