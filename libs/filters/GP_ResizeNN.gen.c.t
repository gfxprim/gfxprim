@ include source.t
/*
 * Nearest Neighbour resampling
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"

#include "core/GP_Debug.h"

#include "GP_ResizeNN.h"

@ for pt in pixeltypes:
@     if not pt.is_unknown():
static int resize_nn{{ pt.name }}(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	uint32_t xmap[dst->w];
	uint32_t ymap[dst->h];
	uint32_t i;
	GP_Coord x, y;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	/* Pre-compute mapping for interpolation */
	for (i = 0; i < dst->w; i++)
		xmap[i] = ((((i * (src->w - 1))<<8)) / (dst->w - 1) + (1<<7))>>8;

	for (i = 0; i < dst->h; i++)
		ymap[i] = ((((i * (src->h - 1))<<8) + (dst->h - 1)/2) / (dst->h - 1) + (1<<7))>>8;

	/* Interpolate */
	for (y = 0; y < (GP_Coord)dst->h; y++) {
		for (x = 0; x < (GP_Coord)dst->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, xmap[x], ymap[y]);

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, dst->h, dst->w)) {
			errno = ECANCELED;
			return 1;
		}
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int resize_nn(const GP_Context *src, GP_Context *dst,
                     GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		return resize_nn{{ pt.name }}(src, dst, callback);
	break;
@ end
	default:
		return -1;
	}
}

int GP_FilterResizeNN(const GP_Context *src, GP_Context *dst,
                           GP_ProgressCallback *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize_nn(src, dst, callback);
}
