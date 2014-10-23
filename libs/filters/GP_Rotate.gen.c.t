@ include source.t
/*
 * Vertical Mirror alogorithm
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Debug.h"
#include "core/GP_GetPutPixel.h"
#include "GP_Rotate.h"

@ for ps in pixelsizes:
static int GP_FilterRotate90_Raw_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst,
                                          GP_ProgressCallback *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 90 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
			uint32_t yr = src->h - y - 1;
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, yr, x, GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y));
		}

		if (GP_ProgressCallbackReport(callback, x, src->w, src->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int GP_FilterRotate90_Raw(const GP_Context *src, GP_Context *dst,
                                 GP_ProgressCallback *callback)
{
	GP_FN_RET_PER_BPP_CONTEXT(GP_FilterRotate90_Raw, src, src, dst, callback);
	return 1;
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

@ def swap_pixels(ps, src, dst, x0, y0, x1, y1):
GP_Pixel pix0 = GP_GetPixel_Raw_{{ ps.suffix }}({{ src }}, {{ x0 }}, {{ y0 }});
GP_Pixel pix1 = GP_GetPixel_Raw_{{ ps.suffix }}({{ src }}, {{ x1 }}, {{ y1 }});
GP_PutPixel_Raw_{{ ps.suffix }}({{ dst }}, {{ x0 }}, {{ y0 }}, pix1);
GP_PutPixel_Raw_{{ ps.suffix }}({{ dst }}, {{ x1 }}, {{ y1 }}, pix0);
@ end
@
@ for ps in pixelsizes:
static int GP_FilterRotate180_Raw_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst,
                                          GP_ProgressCallback *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 180 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
			uint32_t xr = src->w - x - 1;
			uint32_t yr = src->h - y - 1;

			{@ swap_pixels(ps, 'src', 'dst', 'x', 'y', 'xr', 'yr') @}
		}

		if (GP_ProgressCallbackReport(callback, x, src->w, src->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int GP_FilterRotate180_Raw(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	GP_FN_RET_PER_BPP_CONTEXT(GP_FilterRotate180_Raw, src, src, dst, callback);
	return 1;
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

@ for ps in pixelsizes:
static int GP_FilterRotate270_Raw_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst,
                                           GP_ProgressCallback *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 270 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
				uint32_t xr = src->w - x - 1;
				GP_PutPixel_Raw_{{ ps.suffix }}(dst, y, xr, GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y));
		}

		if (GP_ProgressCallbackReport(callback, x, src->w, src->h))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int GP_FilterRotate270_Raw(const GP_Context *src, GP_Context *dst,
                                  GP_ProgressCallback *callback)
{
	GP_FN_RET_PER_BPP_CONTEXT(GP_FilterRotate270_Raw, src, src, dst, callback);
	return 1;
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
