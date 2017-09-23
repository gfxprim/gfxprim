@ include source.t
/*
 * Horizontal Mirror alogorithm
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Rotate.h"

@ for ps in pixelsizes:
static int GP_MirrorH_Raw_{{ ps.suffix }}(const GP_Pixmap *src,
                                    GP_Pixmap *dst,
                                    GP_ProgressCallback *callback)
{
	uint32_t x, y;
	GP_Pixel tmp;

	GP_DEBUG(1, "Mirroring image %ux%u horizontally", src->w, src->h);

	for (x = 0; x < src->w/2; x++) {
		uint32_t xm = src->w - x - 1;
		for (y = 0; y < src->h; y++) {
			tmp = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, GP_GetPixel_Raw_{{ ps.suffix }}(src, xm, y));
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, xm, y, tmp);
		}

		if (GP_ProgressCallbackReport(callback, 2 * x, src->w, src->h))
			return 1;
	}

	/* Copy the middle odd line */
	if (src != dst && src->w % 2) {
		x = src->w / 2;
		for (y = 0; y < src->h; y++)
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y));
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int GP_FilterMirrorH_Raw(const GP_Pixmap *src, GP_Pixmap *dst,
                                GP_ProgressCallback *callback)
{
	GP_FN_RET_PER_BPP_PIXMAP(GP_MirrorH_Raw, src, src, dst, callback);
	return 1;
}

int GP_FilterMirrorH(const GP_Pixmap *src, GP_Pixmap *dst,
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

GP_Pixmap *GP_FilterMirrorHAlloc(const GP_Pixmap *src,
                                   GP_ProgressCallback *callback)
{
	GP_Pixmap *res;

	res = GP_PixmapCopy(src, 0);

	if (res == NULL)
		return NULL;

	if (GP_FilterMirrorH_Raw(src, res, callback)) {
		GP_PixmapFree(res);
		return NULL;
	}

	return res;
}
