%% extends "base.c.t"

{% macro maybe_opts(opts) %}{% if opts %}, {{ opts }}{% endif %}{% endmacro %}

{% macro maybe_opts2(opts) %}{% if opts %}{{ opts }}, {% endif %}{% endmacro %}

%% macro filter_include()
#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Point.h"
#include "GP_Filter.h"
%% endmacro

/*
 * Value clamping macro
 */
%% macro filter_clamp_val(var, size)
			if ({{ var }} < 0)
				{{ var }} = 0;

			if ({{ var }} > {{ 2 ** size - 1}})
				{{ var }} = {{ 2 ** size - 1}};
%% endmacro 

/*
 * Load parameters from params structure into variables
 */
%% macro filter_params(pt, params, c_type, suffix, id)
	GP_ASSERT(GP_FilterParamCheckPixelType({{ params }}, GP_PIXEL_{{ pt.name }}) == 0,
	          "Invalid params channels for context pixel type");
	
	%% for chann in pt.chanslist
	{{ c_type }}{{ chann[0] }}{{ suffix }} = (GP_FilterParamChannel({{ params }}, "{{ chann[0] }}"))->val.{{ id }};
	%% endfor
%% endmacro

%% macro filter_params_raw(pt, params, suffix)
	GP_ASSERT(GP_FilterParamCheckPixelType({{ params }}, GP_PIXEL_{{ pt.name }}) == 0,
	          "Invalid params channels for context pixel type");
	
	%% for chann in pt.chanslist
	GP_FilterParam *{{ chann[0] }}{{ suffix }} = GP_FilterParamChannel({{ params }}, "{{ chann[0] }}");
	%% endfor
%% endmacro

/*
 * Filter per pixel type, used for images with more than one channel per pixel
 */
%% macro filter_point_per_channel(name, opts="", filter_op)
%% for pt in pixeltypes
%% if not pt.is_unknown() and len(pt.chanslist) > 1
static int GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src, GP_Context *dst,
	{{ maybe_opts2(opts) }}GP_ProgressCallback *callback)
{
{{ caller(pt) }}
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
			%% for c in pt.chanslist
			int32_t {{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix);
			%% endfor

			%% for c in pt.chanslist
			{{ filter_op(c[0], c[2]) }}
			%% endfor

			pix = GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

%% endif
%% endfor
%% endmacro

/*
 * Load parameters from params structure into variable
 */
%% macro filter_param(ps, params, c_type, suffix, id)
	GP_ASSERT(GP_FilterParamChannels({{ params }}) != 1,
	          "Expected only one channel");

	{{ c_type }}pix{{ suffix }} = {{ params }}[0].val.{{ id }};
%% endmacro

%% macro filter_param_raw(ps, params, suffix)
	GP_ASSERT(GP_FilterParamChannels({{ params }}) != 1,
	          "Expected only one channel");

	GP_FilterParam *pix{{ suffix }} = &{{ params }}[0];
%% endmacro

/*
 * Point filter per bpp (used for 1 channel pixels to save space).
 */
%% macro filter_point_per_bpp(name, opts="", filter_op)
%% for ps in pixelsizes
%% if ps.size <= 8 and ps.size > 1
static int GP_Filter{{ name }}_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst,
	{{ maybe_opts2(opts) }}GP_ProgressCallback *callback)
{
{{ caller(ps) }}
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			int32_t pix = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			{{ filter_op('pix', ps.size) }}
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, pix);
		}
		
		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
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
%% macro filter_functions(name, opts="", params="", fmt="")
int GP_Filter{{ name }}_Raw(const GP_Context *src, GP_Context *dst{{ maybe_opts(opts) }},
	GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Running filter {{ name }}");

	switch (src->pixel_type) {
	%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
		%% if pt.is_unknown() or pt.pixelsize.size < 2
		return 1;
		%% elif len(pt.chanslist) == 1:
		//TODO: BITENDIAN
		return GP_Filter{{ name }}_{{ pt.pixelsize.suffix }}(src, dst{{ maybe_opts(params) }}, callback);
		%% else
		return GP_Filter{{ name }}_{{ pt.name }}(src, dst{{ maybe_opts(params) }}, callback);
		%% endif
	%% endfor
	default:
	break;
	}

	return 1;
}

GP_Context *GP_Filter{{ name }}(const GP_Context *src, GP_Context *dst{{ maybe_opts(opts) }},
	GP_ProgressCallback *callback)
{
	GP_Context *res = dst;

	if (res == NULL) {
		res = GP_ContextCopy(src, 0);

		if (res == NULL)
			return NULL;
	} else {
		GP_ASSERT(src->pixel_type == dst->pixel_type,
		          "The src and dst pixel types must match");
		GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
		          "Destination is not big enough");
	}

	if (GP_Filter{{ name }}_Raw(src, dst{{ maybe_opts(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");

		if (dst == NULL)
			GP_ContextFree(res);
	
		return NULL;
	}

	return res;
}
%% endmacro
