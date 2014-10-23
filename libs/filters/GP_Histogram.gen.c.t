@ include source.t
/*
 * Histogram filter -- Compute image histogram
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Filter.h"

#include "GP_Stats.h"

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int GP_FilterHistogram_{{ pt.name }}(const GP_Context *src,
        GP_FilterParam histogram[], GP_ProgressCallback *callback)
{
	GP_ASSERT(GP_FilterParamCheckPixelType(histogram, GP_PIXEL_{{ pt.name }}) == 0,
	          "Invalid params channels for context pixel type");

@         for c in pt.chanslist:
	GP_Histogram *{{ c.name }}_hist = (GP_FilterParamChannel(histogram, "{{ c.name }}"))->val.ptr;
@         end

	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
@         for c in pt.chanslist:
			int32_t {{ c.name }} = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
@         end

@         for c in pt.chanslist:
			{{ c.name }}_hist->hist[{{ c.name }}]++;
@         end
		}

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
int GP_FilterHistogram_Raw(const GP_Context *src, GP_FilterParam histogram[],
                           GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Running filter Histogram");

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterHistogram_{{ pt.name }}(src, histogram, callback);
@ end
	default:
	break;
	}

	return 1;
}
