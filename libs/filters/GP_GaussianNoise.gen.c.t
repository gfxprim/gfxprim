%% extends "filter.c.t"

{% block descr %}Gaussian Noise{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_TempAlloc.h"
//#include "core/GP_Gamma.h"
#include "core/GP_Clamp.h"
#include "core/GP_Debug.h"

#include "GP_Rand.h"
#include "GP_GaussianNoise.h"

%% for pt in pixeltypes
%% if not pt.is_unknown() and not pt.is_palette()
static int GP_FilterGaussianNoiseAdd_{{ pt.name }}_Raw(const GP_Context *src,
                                GP_Coord x_src, GP_Coord y_src,
                                GP_Size w_src, GP_Size h_src,
                                GP_Context *dst,
                                GP_Coord x_dst, GP_Coord y_dst,
                                float sigma, float mu,
                                GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Additive Gaussian noise filter %ux%u sigma=%f mu=%f",
	         w_src, h_src, sigma, mu);
	
	%% for c in pt.chanslist
	int sigma_{{ c[0] }} = {{ 2 ** c[2] - 1 }} * sigma;
	int mu_{{ c[0] }} = {{ 2 ** c[2] - 1 }} * mu;
	%% endfor

	unsigned int size = w_src + w_src%2;

	/* Create temporary buffers */
	GP_TempAllocCreate(temp, sizeof(int) * size * {{ len(pt.chanslist) }});

	%% for c in pt.chanslist
	int *{{ c[0] }} = GP_TempAllocGet(temp, size * sizeof(int));
	%% endfor
	
	/* Apply the additive noise filter */
	unsigned int x, y;

	for (y = 0; y < h_src; y++) {
		%% for c in pt.chanslist
		GP_NormInt({{ c[0] }}, size, sigma_{{ c[0] }}, mu_{{ c[0] }});
		%% endfor
			
		for (x = 0; x < w_src; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x + x_src, y + y_src);
			
			%% for c in pt.chanslist
			{{ c[0] }}[x] += GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix);
			{{ c[0] }}[x] = GP_CLAMP({{ c[0] }}[x], 0, {{ 2 ** c[2] - 1 }});
			%% endfor
			
			pix = GP_Pixel_CREATE_{{ pt.name }}({{ expand_chanslist(pt, "", "[x]") }});
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x + x_dst, y + y_dst, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, h_src, w_src)) {
			GP_TempAllocFree(temp);
			return 1;
		}
	}
	
	GP_TempAllocFree(temp);
	GP_ProgressCallbackDone(callback);

	return 0;
}

%% endif
%% endfor

int GP_FilterGaussianNoiseAdd_Raw(const GP_Context *src,
                                  GP_Coord x_src, GP_Coord y_src,
                                  GP_Size w_src, GP_Size h_src,
                                  GP_Context *dst,
                                  GP_Coord x_dst, GP_Coord y_dst,
                                  float sigma, float mu,
                                  GP_ProgressCallback *callback)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	%% if not pt.is_unknown() and not pt.is_palette()
	case GP_PIXEL_{{ pt.name }}:
		return GP_FilterGaussianNoiseAdd_{{ pt.name }}_Raw(src, x_src,
				y_src, w_src, h_src, dst, x_dst, y_dst,
				sigma, mu, callback);
	break;
	%% endif
	%% endfor
	default:
		return -1;
	}
}

int GP_FilterGaussianNoiseAddEx(const GP_Context *src,
                                GP_Coord x_src, GP_Coord y_src,
                                GP_Size w_src, GP_Size h_src,
                                GP_Context *dst,
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

GP_Context *GP_FilterGaussianNoiseAddExAlloc(const GP_Context *src,
                                             GP_Coord x_src, GP_Coord y_src,
                                             GP_Size w_src, GP_Size h_src,
                                             float sigma, float mu,
                                             GP_ProgressCallback *callback)
{
	int ret;

	GP_Context *dst = GP_ContextAlloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = GP_FilterGaussianNoiseAdd_Raw(src, x_src, y_src, w_src, h_src,
	                                    dst, 0, 0, sigma, mu, callback);

	if (ret) {
		GP_ContextFree(dst);
		return NULL;
	}

	return dst;
}

%% endblock body
