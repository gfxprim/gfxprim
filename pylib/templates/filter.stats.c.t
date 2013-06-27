%% extends "filter.c.t"

%% macro filter_stats_include()
{{ filter_include() }}
#include "GP_Stats.h"
%% endmacro

/*
 * Filter per pixel type, used for images with more than one channel per pixel
 */
%% macro filter_point_per_channel(name, opts="", filter_op)
%% for pt in pixeltypes
%% if not pt.is_unknown() and len(pt.chanslist) > 1
static int GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src,
	{{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
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
 * Point filter per bpp (used for 1 channel pixels to save space).
 */
%% macro filter_point_per_bpp(name, opts="", filter_op)
%% for ps in pixelsizes
%% if ps.size > 1
static int GP_Filter{{ name }}_{{ ps.suffix }}(const GP_Context *src,
	{{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
{
{{ caller(ps) }}
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			int32_t pix = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			{{ filter_op('pix', ps.size) }}
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
int GP_Filter{{ name }}_Raw(const GP_Context *src{{ maybe_opts_l(opts) }},
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
		return GP_Filter{{ name }}_{{ pt.pixelsize.suffix }}(src{{ maybe_opts_l(params) }}, callback);
		%% else
		return GP_Filter{{ name }}_{{ pt.name }}(src{{ maybe_opts_l(params) }}, callback);
		%% endif
	%% endfor
	default:
	break;
	}

	return 1;
}

%% endmacro
