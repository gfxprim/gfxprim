@ include source.t
/*
 * Hilbert Peano dithering RGB888 -> any pixel
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include "core/GP_Core.h"
#include "core/GP_GetPutPixel.h"
#include "filters/GP_HilbertCurve.h"
#include "filters/GP_Filter.h"
#include "filters/GP_Dither.h"

/*
 * Returns closest greater square of two, used to determine the curve size.
 */
static unsigned int count_bits(unsigned int n)
{
	unsigned int i = 0, s = n;

	do {
		n>>=1;
		i++;
	} while (n);

	i--;

	return (i + (s != (1U<<i)));
}

@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
/*
 * Hilbert Peano to {{ pt.name }}
 */
static int hilbert_peano_to_{{ pt.name }}_Raw(const GP_Context *src,
                                              GP_Context *dst,
                                              GP_ProgressCallback *callback)
{
	struct GP_CurveState state;
	unsigned int n;

	n = GP_MAX(count_bits(src->w), count_bits(src->h));

	GP_DEBUG(1, "Hilbert Peano dithering %ux%u -> n = %u", src->w, src->h, n);

	GP_HilbertCurveInit(&state, n);

	/* processed pixels counter */
	unsigned int cnt = 0;

	/* error counters */
@         for c in pt.chanslist:
	int err_{{ c[0] }} = 0;
@         end

	while (GP_HilbertCurveContinues(&state)) {
		if (state.x < src->w && state.y < src->h) {
			GP_Pixel pix;

			pix = GP_GetPixel_Raw(src, state.x, state.y);
			pix = GP_PixelToRGB888(pix, src->pixel_type);

@         for c in pt.chanslist:
@             if pt.is_gray():
			int pix_{{ c[0] }} = GP_Pixel_GET_R_RGB888(pix) +
			                     GP_Pixel_GET_G_RGB888(pix) +
			                     GP_Pixel_GET_B_RGB888(pix);
@             else:
			int pix_{{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_RGB888(pix);
@             end
			pix_{{ c[0] }} += err_{{ c[0] }};

@             if pt.is_gray():
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 382) / {{ 3 * 255 }};
			err_{{ c[0] }} = pix_{{ c[0] }} - {{ 3 * 255 }} * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
@             else:
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 127) / 255;
			err_{{ c[0] }} = pix_{{ c[0] }} - 255 * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
@         end

@         if pt.is_gray():
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res_V);
@         else:
			GP_Pixel res = GP_Pixel_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res);
@         end
			cnt++;

			if (GP_ProgressCallbackReport(callback, cnt/src->h, src->w, src->h))
				return 1;

			/* We are done, exit */
			if (cnt == src->w * src->h - 1) {
				GP_ProgressCallbackDone(callback);
				return 0;
			}
		} else {
@         for c in pt.chanslist:
			err_{{ c[0] }} = 0;
@         end
		}

		GP_HilbertCurveNext(&state);
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
static int hilbert_peano(const GP_Context *src, GP_Context *dst,
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
		return hilbert_peano_to_{{ pt.name }}_Raw(src, dst, callback);
@ end
	default:
		errno = EINVAL;
		return 1;
	}
}

int GP_FilterHilbertPeano(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return hilbert_peano(src, dst, callback);
}

GP_Context *GP_FilterHilbertPeanoAlloc(const GP_Context *src,
                                       GP_PixelType pixel_type,
                                       GP_ProgressCallback *callback)
{
	GP_Context *ret;

	ret = GP_ContextAlloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (hilbert_peano(src, ret, callback)) {
		GP_ContextFree(ret);
		return NULL;
	}

	return ret;
}
