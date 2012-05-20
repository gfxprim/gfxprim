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

int GP_FilterMirrorH_Raw(const GP_Context *src, GP_Context *dst,
                         GP_ProgressCallback *callback)
{
	uint32_t bpr = src->bytes_per_row;
	uint8_t  buf[bpr];
	uint32_t y;

	GP_DEBUG(1, "Mirroring image horizontally %ux%u", src->w, src->h);

	#warning FIXME: non byte aligned pixels
	
	/* Note that this should work both for src != dst and src == dst */
	for (y = 0; y < src->h/2; y++) {
		uint8_t *sl1 = src->pixels + bpr * y;
		uint8_t *sl2 = src->pixels + bpr * (src->h - y - 1);
		uint8_t *dl1 = dst->pixels + bpr * y;
		uint8_t *dl2 = dst->pixels + bpr * (src->h - y - 1);

		memcpy(buf, sl1, bpr);
		memcpy(dl1, sl2, bpr);
		memcpy(dl2, buf, bpr);
		
		if (GP_ProgressCallbackReport(callback, 2 * y, src->h, src->w))
			return 1;
	}

	/* Copy the middle odd line */
	if (src != dst && src->h % 2) {
		y = src->h / 2;

		uint8_t *sl = src->pixels + bpr * y;
		uint8_t *dl = dst->pixels + bpr * y;
		
		memcpy(dl, sl, bpr);
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

int GP_FilterMirrorH(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
		  "The src and dst pixel types must match");
	
	GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
	          "Destination is not large enough");

	if (GP_FilterMirrorH_Raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

GP_Context *GP_FilterMirrorHAlloc(const GP_Context *src,
                                  GP_ProgressCallback *callback)
{
	GP_Context *res;
	
	res = GP_ContextCopy(src, 0);
		
	if (res == NULL)
		return NULL;

	if (GP_FilterMirrorH_Raw(src, res, callback)) {
		GP_ContextFree(res);
		return NULL;
	}
	
	return res;
}

int GP_FilterMirrorV(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
	          "Destination is not large enough");

	if (GP_FilterMirrorV_Raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

GP_Context *GP_FilterMirrorVAlloc(const GP_Context *src,
                                  GP_ProgressCallback *callback)
{
	GP_Context *res;
	
	res = GP_ContextCopy(src, 0);
		
	if (res == NULL)
		return NULL;

	if (GP_FilterMirrorV_Raw(src, res, callback)) {
		GP_ContextFree(res);
		return NULL;
	}
	
	return res;
}

int GP_FilterRotate180_Raw(const GP_Context *src, GP_Context *dst,
                           GP_ProgressCallback *callback)
{
	#warning FIXME: Callbacks, faster algorighm?

	GP_FilterMirrorV_Raw(src, dst, NULL);
	GP_FilterMirrorH_Raw(dst, dst, callback);

	return 0;
}

int GP_FilterRotate90(const GP_Context *src, GP_Context *dst,
                      GP_ProgressCallback *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
	          "Destination is not large enough");

	if (GP_FilterRotate90_Raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

GP_Context *GP_FilterRotate90Alloc(const GP_Context *src,
                                   GP_ProgressCallback *callback)
{
	GP_Context *res;
		
	res = GP_ContextAlloc(src->h, src->w, src->pixel_type);
	
	if (res == NULL)
		return NULL;

	if (GP_FilterRotate90_Raw(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		GP_ContextFree(res);
		return NULL;
	}

	return res;
}

int GP_FilterRotate180(const GP_Context *src, GP_Context *dst,
                       GP_ProgressCallback *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
	          "Destination is not large enough");

	if (GP_FilterRotate180_Raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

GP_Context *GP_FilterRotate180Alloc(const GP_Context *src,
                                    GP_ProgressCallback *callback)
{
	GP_Context *res;
		
	res = GP_ContextCopy(src, 0);
	
	if (res == NULL)
		return NULL;

	if (GP_FilterRotate180_Raw(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		GP_ContextFree(res);
		return NULL;
	}

	return res;
}

int GP_FilterRotate270(const GP_Context *src, GP_Context *dst,
                      GP_ProgressCallback *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
	          "Destination is not large enough");

	if (GP_FilterRotate270_Raw(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

GP_Context *GP_FilterRotate270Alloc(const GP_Context *src,
                                    GP_ProgressCallback *callback)
{
	GP_Context *res;
		
	res = GP_ContextAlloc(src->h, src->w, src->pixel_type);
	
	if (res == NULL)
		return NULL;

	if (GP_FilterRotate270_Raw(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		GP_ContextFree(res);
		return NULL;
	}

	return res;
}

static const char *symmetry_names[] = {
	"90",
	"180",
	"270",
	"H",
	"V",
	NULL,
};

const char **GP_FilterSymmetryNames = symmetry_names;

int GP_FilterSymmetryByName(const char *symmetry)
{
	int i;

	for (i = 0; symmetry_names[i] != NULL; i++)
		if (!strcasecmp(symmetry, symmetry_names[i]))
			return i;

	return -1;
}

GP_Context *GP_FilterSymmetryAlloc(const GP_Context *src,
                                   GP_FilterSymmetries symmetry,
			           GP_ProgressCallback *callback)
{
	switch (symmetry) {
	case GP_ROTATE_90:
		return GP_FilterRotate90Alloc(src, callback);
	case GP_ROTATE_180:
		return GP_FilterRotate180Alloc(src, callback);
	case GP_ROTATE_270:
		return GP_FilterRotate270Alloc(src, callback);
	case GP_MIRROR_H:
		return GP_FilterMirrorHAlloc(src, callback);
	case GP_MIRROR_V:
		return GP_FilterMirrorVAlloc(src, callback);
	default:
		GP_DEBUG(1, "Invalid symmetry %i", (int) symmetry);
		return NULL;
	}
}

int GP_FilterSymmetry(const GP_Context *src, GP_Context *dst,
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
	default:
		GP_DEBUG(1, "Invalid symmetry %i", (int) symmetry);
		return 1;
	}
}
