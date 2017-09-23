@ include source.t
/*
 * Gaussian Noise
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include "core/GP_Pixmap.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
//#include "core/GP_Gamma.h"
#include "core/GP_Clamp.h"
#include "core/GP_Debug.h"

#include "GP_Rand.h"
#include "GP_GaussianNoise.h"

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int GP_FilterGaussianNoiseAdd_{{ pt.name }}_Raw(const GP_Pixmap *src,
                                GP_Coord x_src, GP_Coord y_src,
                                GP_Size w_src, GP_Size h_src,
                                GP_Pixmap *dst,
                                GP_Coord x_dst, GP_Coord y_dst,
                                float sigma, float mu,
                                GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Additive Gaussian noise filter %ux%u sigma=%f mu=%f",
	         w_src, h_src, sigma, mu);

@         for c in pt.chanslist:
	int sigma_{{ c.name }} = {{ c.max }} * sigma;
	int mu_{{ c.name }} = {{ c.max }} * mu;
@         end

	unsigned int size = w_src + w_src%2;

	/* Create temporary buffers */
	GP_TempAllocCreate(temp, sizeof(int) * size * {{ len(pt.chanslist) }});

@         for c in pt.chanslist:
	int *{{ c.name }} = GP_TempAllocGet(temp, size * sizeof(int));
@         end

	/* Apply the additive noise filter */
	unsigned int x, y;

	for (y = 0; y < h_src; y++) {
@         for c in pt.chanslist:
		GP_NormInt({{ c.name }}, size, sigma_{{ c.name }}, mu_{{ c.name }});
@         end

		for (x = 0; x < w_src; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x + x_src, y + y_src);

@         for c in pt.chanslist:
			{{ c.name }}[x] += GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
			{{ c.name }}[x] = GP_CLAMP({{ c.name }}[x], 0, {{ c.max }});
@         end

			pix = GP_Pixel_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, '', '[x]') }});
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x + x_dst, y + y_dst, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(temp);
			errno = ECANCELED;
			return 1;
		}
	}

	GP_TempAllocFree(temp);
	GP_ProgressCallbackDone(callback);

	return 0;
}

@ end
@
int GP_FilterGaussianNoiseAdd_Raw(const GP_Pixmap *src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  GP_Pixmap *dst,
                                  GP_Coord x_dst, GP_Coord y_dst,
                                  float sigma, float mu,
                                  GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterGaussianNoiseAdd_{{ pt.name }}_Raw(src, x_src,
				y_src, w_src, h_src, dst, x_dst, y_dst,
				sigma, mu, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

int GP_FilterGaussianNoiseAddEx(const GP_Pixmap *src,
                                GP_Coord x_src, GP_Coord y_src,
                                GP_Size w_src, GP_Size h_src,
                                GP_Pixmap *dst,
                                GP_Coord x_dst, GP_Coord y_dst,
                                float sigma, float mu,
                                GP_ProgressCallback *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (GP_Coord)w_src <= (GP_Coord)dst->w);
	GP_CHECK(y_dst + (GP_Coord)h_src <= (GP_Coord)dst->h);

	/* Source is large enough */
	GP_CHECK(x_src + w_src <= src->w);
	GP_CHECK(y_src + h_src <= src->h);

	return GP_FilterGaussianNoiseAdd_Raw(src, x_src, y_src, w_src, h_src,
	                                     dst, x_dst, y_dst,
	                                     sigma, mu, callback);
}

GP_Pixmap *GP_FilterGaussianNoiseAddExAlloc(const GP_Pixmap *src,
                                             GP_Coord x_src, GP_Coord y_src,
                                             GP_Size w_src, GP_Size h_src,
                                             float sigma, float mu,
                                             GP_ProgressCallback *callback)
{
	int ret, err;

	GP_Pixmap *dst = GP_PixmapAlloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = GP_FilterGaussianNoiseAdd_Raw(src, x_src, y_src, w_src, h_src,
	                                    dst, 0, 0, sigma, mu, callback);

	if (ret) {
		err = errno;
		GP_PixmapFree(dst);
		errno = err;
		return NULL;
	}

	return dst;
}
