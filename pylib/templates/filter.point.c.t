%% extends "base.c.t"

{% macro maybe_opts(opts) %}{% if opts %}, {{ opts }}{% endif %}{% endmacro %}

%% macro filter_include()
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>
#include <GP_Debug.h>
%% endmacro

/*
 * Filter per pixel size, used for one channel images.
 */
%% macro filter_per_pixel_size(name, opts="")
%% for ps in pixelsizes
%% if ps.size <= 8 and ps.size > 1
void GP_Filter{{ name }}_{{ ps.suffix }}(const GP_Context *src, GP_Context *res{{ maybe_opts(opts) }})
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			int32_t pix = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			{{ caller(ps) }}
			GP_PutPixel_Raw_{{ ps.suffix }}(res, x, y, pix);
		}
	}
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
void GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src, GP_Context *res{{ maybe_opts(opts) }})
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++)
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
			%% for c in pt.chanslist
			int32_t {{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix);
			%% endfor

			%% for c in pt.chanslist
			{{ caller(c) }}
			%% endfor

			pix = GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %});
		
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(res, x, y, pix);
		}
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
void GP_Filter{{ name }}_Raw(const GP_Context *src, GP_Context *res{{ maybe_opts(opts) }})
{
	GP_DEBUG(1, "Running filter {{ name }} {{ fmt }}"{{ maybe_opts(params) }});

	switch (src->pixel_type) {
	%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
		%% if pt.is_unknown() or pt.pixelsize.size < 2
		return;
		%% elif len(pt.chanslist) == 1:
		GP_Filter{{ name }}_{{ pt.pixelsize.suffix }}(src, res{{ maybe_opts(params) }});
		%% else
		GP_Filter{{ name }}_{{ pt.name }}(src, res{{ maybe_opts(params) }});
		%% endif
	break;
	%% endfor
	default:
	break;
	}
}

GP_Context *GP_Filter{{ name }}(const GP_Context *src{{ maybe_opts(opts) }})
{
	GP_Context *res;

	res = GP_ContextCopy(src, 0);

	if (res == NULL)
		return NULL;

	GP_Filter{{ name }}_Raw(src, res{{ maybe_opts(params) }});

	return res;
}
%% endmacro

