%% extends "filter.c.t"

%% macro filter_arithmetic_include()
{{ filter_include() }}
#include "GP_Arithmetic.h"
%% endmacro

/*
 * Filter per pixel type, used for images with more than one channel per pixel
 */
%% macro filter_arithmetic_per_channel(name, filter_op, opts="")
%% for pt in pixeltypes
%% if not pt.is_unknown() and len(pt.chanslist) > 1
static int GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst, {{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
{
{{ caller(pt) }}
	uint32_t x, y, w, h;

	w = GP_MIN(src_a->w, src_b->w);
	h = GP_MIN(src_a->h, src_b->h);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GP_Pixel pix_a = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src_a, x, y);
			GP_Pixel pix_b = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src_b, x, y);

			%% for c in pt.chanslist
			int32_t {{ c[0] }}_A = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix_a);
			int32_t {{ c[0] }}_B = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix_b);
			%% endfor

			%% for c in pt.chanslist
			int32_t {{ c[0] }};
			{{ filter_op(c[0], c[2]) }}
			%% endfor

			GP_Pixel pix;
			pix = GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, y, h, w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}
%% endif
%% endfor
%% endmacro

/*
 * Point filter per bpp (used for 1 channel pixels to save space).
 */
%% macro filter_arithmetic_per_bpp(name, filter_op, opts="")
%% for ps in pixelsizes
%% if ps.size <= 8 and ps.size > 1
static int GP_Filter{{ name }}_{{ ps.suffix }}(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst, {{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
{
{{ caller(ps) }}
	uint32_t x, y, w, h;
	
	w = GP_MIN(src_a->w, src_b->w);
	h = GP_MIN(src_a->h, src_b->h);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			int32_t pix_A = GP_GetPixel_Raw_{{ ps.suffix }}(src_a, x, y);
			int32_t pix_B = GP_GetPixel_Raw_{{ ps.suffix }}(src_b, x, y);
			int32_t pix;
			{{ filter_op('pix', ps.size) }}
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, y, h, w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endif
%% endfor
%% endmacro

/*
 * Switch per pixel sizes or pixel types.
 */
%% macro filter_arithmetic_functions(name, opts="", params="", fmt="")
int GP_Filter{{ name }}_Raw(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst{{ maybe_opts_l(opts) }}, GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Running filter {{ name }}");

	switch (src_a->pixel_type) {
	%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
		%% if pt.is_unknown() or pt.pixelsize.size < 2
		return 1;
		%% elif len(pt.chanslist) == 1:
		//TODO: BITENDIAN
		return GP_Filter{{ name }}_{{ pt.pixelsize.suffix }}(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback);
		%% else
		return GP_Filter{{ name }}_{{ pt.name }}(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback);
		%% endif
	%% endfor
	default:
	break;
	}

	return 1;
}

GP_Context *GP_Filter{{ name }}(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst{{ maybe_opts_l(opts) }}, GP_ProgressCallback *callback)
{
	GP_Context *res = dst;

	GP_ASSERT(src_a->pixel_type == src_b->pixel_type,
	          "Pixel types for sources must match.");

	GP_Size w = GP_MIN(src_a->w, src_b->w);
	GP_Size h = GP_MIN(src_a->h, src_b->h);
	
	if (res == NULL) {
		
		res = GP_ContextAlloc(w, h, src_a->pixel_type);

		if (res == NULL)
			return NULL;
	} else {
		GP_ASSERT(src_a->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(w <= dst->w && h <= dst->h,
		          "Destination is not big enough");
	}

	if (GP_Filter{{ name }}_Raw(src_a, src_b, res{{ maybe_opts_l(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");

		if (dst == NULL)
			GP_ContextFree(res);
	
		return NULL;
	}

	return res;
}
%% endmacro
