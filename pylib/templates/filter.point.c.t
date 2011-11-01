%% extends "base.c.t"

{% macro maybe_opts(opts) %}{% if opts %}, {{ opts }}{% endif %}{% endmacro %}

%% macro filter_include()
#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Filter.h"
%% endmacro

/*
 * Filter per pixel size, used for one channel images.
 */
%% macro filter_per_pixel_size(name, opts="")
%% for ps in pixelsizes
%% if ps.size <= 8 and ps.size > 1
int GP_Filter{{ name }}_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst{{ maybe_opts(opts) }},
	GP_ProgressCallback *callback)
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			int32_t pix = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			{{ caller(ps) }}
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
 * Filter per pixel type, used for images with more than one channel per pixel
 */
%% macro filter_per_pixel_type(name, opts="")
%% for pt in pixeltypes
%% if not pt.is_unknown() and len(pt.chanslist) > 1
int GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src, GP_Context *dst{{ maybe_opts(opts) }},
	GP_ProgressCallback *callback)
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
			%% for c in pt.chanslist
			int32_t {{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix);
			%% endfor

			%% for c in pt.chanslist
			{{ caller(c) }}
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
 * Value clamping macro
 */
%% macro filter_clamp_val(var, size)
			if ({{ var }} < 0)
				{{ var }} = 0;

			if ({{ var }} > {{ 2 ** size - 1}})
				{{ var }} = {{ 2 ** size - 1}};
%% endmacro 

/*
 * Switch per pixel sizes or pixel types.
 */
%% macro filter_functions(name, opts="", params="", fmt="")
int GP_Filter{{ name }}_Raw(const GP_Context *src, GP_Context *dst{{ maybe_opts(opts) }},
	GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Running filter {{ name }} {{ fmt }}"{{ maybe_opts(params) }});

	switch (src->pixel_type) {
	%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
		%% if pt.is_unknown() or pt.pixelsize.size < 2
		return 1;
		%% elif len(pt.chanslist) == 1:
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

