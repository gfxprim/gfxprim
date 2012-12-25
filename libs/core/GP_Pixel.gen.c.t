%% extends "base.c.t"

%% block descr
Pixel type definitions and functions
%% endblock

%% block body
#include <stdio.h>
#include "GP_Pixel.h"
#include "GP_GetSetBits.h"

/* 
 * Description of all known pixel types 
 */
const GP_PixelTypeDescription const GP_PixelTypes [GP_PIXEL_MAX] = {
%% for pt in pixeltypes
	/* GP_PIXEL_{{ pt.name }} */ {
		.type	= GP_PIXEL_{{ pt.name }},
		.name	= "{{ pt.name }}",
		.size	= {{ pt.pixelsize.size }},
		.bit_endian  = {{ pt.pixelsize.bit_endian_const }},
		.numchannels = {{ len(pt.chanslist) }},
		.bitmap      = "{{ pt.bits|join("") }}",
		.channels    = {
%% for c in pt.chanslist
			{ .name = "{{ c[0] }}", .offset = {{ c[1] }}, .size = {{ c[2] }} },
%% endfor
	} },
%% endfor
};

#warning FIXME: do generic get set bit for pixel printing

%% for pt in pixeltypes
%% if not pt.is_unknown()
/*
 * snprintf a human readable value of pixel type {{pt.name}} 
 */
void GP_PixelSNPrint_{{ pt.name }}(char *buf, size_t len, GP_Pixel p)
{
	snprintf(buf, len, "<{{ pt.name }} 0x%0{{ (pt.pixelsize.size+3)//4 }}x{% for c in pt.chanslist %} {{ c[0] }}=%d{% endfor %}>",
	GP_GET_BITS(0, {{ pt.pixelsize.size }}, p){% for c in pt.chanslist %}, GP_GET_BITS({{ c[1] }}, {{ c[2] }}, p){% endfor %});
}

%% endif
%% endfor
%% endblock body
