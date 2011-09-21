%% extends "base.c.t"

%% block descr
Brightness filters -- Increments all color channels by a fixed value.
%% endblock

%% block body
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>

/*
 * Simple brightness operations for one channel bitmaps
 */
%% for ps in pixelsizes
%% if ps.size <= 8 and ps.size > 1
void GP_FilterBrightness_{{ ps.suffix }}(const GP_Context *src, GP_Context *res, int32_t inc)
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++)
		for (x = 0; x < src->w; x++) {
			int32_t pix = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y) + inc;

			if (pix < 0)
				pix = 0;

			if (pix > {{ 2 ** ps.size - 1 }})
				pix  = {{ 2 ** ps.size - 1}};

			GP_PutPixel_Raw_{{ ps.suffix }}(res, x, y, pix);
		}
}

%% endif
%% endfor

/*
 * Brightness filters for pixel types with more than one channels
 */
%% for pt in pixeltypes
%% if not pt.is_unknown() and len(pt.chanslist) > 1
void GP_FilterBrightness_{{ pt.name }}(const GP_Context *src, GP_Context *res, int32_t inc)
{
	uint32_t x, y;

	for (y = 0; y < src->h; y++)
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
			%% for c in pt.chanslist
			int32_t {{ c[0] }} = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix) + inc;
			%% endfor

			%% for c in pt.chanslist
			if ({{ c[0] }} < 0)
				{{ c[0] }} = 0;
		
			if ({{ c[0] }} > {{ 2 ** c[2] - 1 }})
				{{ c[0] }} = 255;

			%% endfor
			pix = GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %});
		
			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(res, x, y, pix);
		}
}

%% endif
%% endfor

void GP_FilterBrightness_Raw(const GP_Context *src, GP_Context *res, int32_t inc)
{
	switch (src->pixel_type) {
	%% for pt in pixeltypes
	case GP_PIXEL_{{ pt.name }}:
		%% if pt.is_unknown() or pt.pixelsize.size < 2
		return;
		%% elif len(pt.chanslist) == 1:
		GP_FilterBrightness_{{ pt.pixelsize.suffix }}(src, res, inc);
		%% else
		GP_FilterBrightness_{{ pt.name }}(src, res, inc);
		%% endif
	break;
	%% endfor
	default:
	break;
	}
}


%% endblock body
