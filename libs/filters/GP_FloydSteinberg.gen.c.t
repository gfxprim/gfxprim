@ include source.t
/*
 * Floyd Steinberg dithering RGB888 -> any pixel
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */
#include <string.h>
#include <errno.h>

#include "core/GP_Core.h"
#include "core/GP_Pixel.h"
#include "core/GP_Clamp.h"
#include "filters/GP_Filter.h"
#include "filters/GP_Dither.h"

@ def distribute_error(errors, x, y, w, err):
if ({{ x }} + 1 < {{ w }})
	{{ errors }}[{{ y }}%2][{{ x }}+1] += 7 * {{ err }} / 16;

if ({{ x }} > 1)
	{{ errors }}[!({{ y }}%2)][{{ x }}-1] += 3 * {{ err }} / 16;

{{ errors }}[!({{ y }}%2)][{{ x }}] += 5 * {{ err }} / 16;

if ({{ x }} + 1 < {{ w }})
	{{ errors }}[!({{ y }}%2)][{{ x }}+1] += {{ err }} / 16;
@ end
@
@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
/*
 * Floyd Steinberg to {{ pt.name }}
 */
static int floyd_steinberg_to_{{ pt.name }}_Raw(const GP_Pixmap *src,
                                                GP_Pixmap *dst,
                                                GP_ProgressCallback *callback)
{
@         for c in pt.chanslist:
	float errors_{{ c.name }}[2][src->w];
@         end

	GP_DEBUG(1, "Floyd Steinberg %s to %s %ux%u",
	            GP_PixelTypeName(src->pixel_type),
	            GP_PixelTypeName(GP_PIXEL_{{ pt.name }}),
		    src->w, src->h);

	GP_Coord x, y;

@         for c in pt.chanslist:
	memset(errors_{{ c.name }}[0], 0, src->w * sizeof(float));
	memset(errors_{{ c.name }}[1], 0, src->w * sizeof(float));
@         end

	for (y = 0; y < (GP_Coord)src->h; y++) {
		for (x = 0; x < (GP_Coord)src->w; x++) {
			GP_Pixel pix;

			pix = GP_GetPixel_Raw(src, x, y);
			pix = GP_PixelToRGB888(pix, src->pixel_type);

@         for c in pt.chanslist:
@             if pt.is_gray():
			float val_{{ c.name }} = GP_Pixel_GET_R_RGB888(pix) +
			                       GP_Pixel_GET_G_RGB888(pix) +
			                       GP_Pixel_GET_B_RGB888(pix);
@             else:
			float val_{{ c.name }} = GP_Pixel_GET_{{ c.name }}_RGB888(pix);
@             end
			val_{{ c.name }} += errors_{{ c.name }}[y%2][x];

			float err_{{ c.name }} = val_{{ c.name }};
@             if pt.is_gray():
			GP_Pixel res_{{ c.name }} = {{ 2 ** c[2] - 1}} * val_{{ c.name }} / (3 * 255);
			err_{{ c.name }} -= res_{{ c.name }} * (3 * 255) / {{ 2 ** c[2] - 1}};
@             else:
			GP_Pixel res_{{ c.name }} = {{ 2 ** c[2] - 1}} * val_{{ c.name }} / 255;
			err_{{ c.name }} -= res_{{ c.name }} * 255 / {{ 2 ** c[2] - 1}};
@             end

			{@ distribute_error('errors_' + c.name, 'x', 'y', '(GP_Coord)src->w', 'err_' + c.name) @}

			GP_CLAMP_DOWN({{ 'res_' + c.name }}, {{ c.max }});
@         end

@         if pt.is_gray():
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, res_V);
@         else:
			GP_Pixel res = GP_Pixel_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, res);
@         end
		}

@         for c in pt.chanslist:
		memset(errors_{{ c.name }}[y%2], 0, src->w * sizeof(float));
@         end

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int floyd_steinberg(const GP_Pixmap *src, GP_Pixmap *dst,
                           GP_ProgressCallback *callback)
{
	if (GP_PixelHasFlags(src->pixel_type, GP_PIXEL_IS_PALETTE)) {
		GP_DEBUG(1, "Unsupported source pixel type %s",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	switch (dst->pixel_type) {
@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
	case GP_PIXEL_{{ pt.name }}:
		return floyd_steinberg_to_{{ pt.name }}_Raw(src, dst, callback);
@ end
	default:
		errno = EINVAL;
		return 1;
	}
}

int GP_FilterFloydSteinberg(const GP_Pixmap *src, GP_Pixmap *dst,
                            GP_ProgressCallback *callback)
{
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return floyd_steinberg(src, dst, callback);
}


GP_Pixmap *GP_FilterFloydSteinbergAlloc(const GP_Pixmap *src,
                                         GP_PixelType pixel_type,
                                         GP_ProgressCallback *callback)
{
	GP_Pixmap *ret;

	ret = GP_PixmapAlloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (floyd_steinberg(src, ret, callback)) {
		GP_PixmapFree(ret);
		return NULL;
	}

	return ret;
}
