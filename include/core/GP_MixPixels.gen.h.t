%% extends "base.h.t"

%% block descr
Macros to mix two pixels accordingly to percentage.
%% endblock

%% block body


#include "GP_Pixel.h"

%% for pt in pixeltypes
%% if not pt.is_unknown()
/*
 * Mixes two {{ pt.name }} pixels.
 *
 * The percentage is expected as 8 bit unsigned integer [0 .. 255]
 */
#define GP_MIX_PIXELS_{{ pt.name }}(pix1, pix2, perc) ({ \
%% for c in pt.chanslist
	GP_Pixel {{ c[0] }}; \
\
	{{ c[0] }}  = GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix1) * (perc); \
	{{ c[0] }} += GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(pix2) * (255 - (perc)); \
	{{ c[0] }} = ({{ c[0] }} + 128) / 255; \
\
%% endfor
\
	GP_Pixel_CREATE_{{ pt.name }}({{ pt.chanslist[0][0] }}{% for c in pt.chanslist[1:] %}, {{ c[0] }}{% endfor %}); \
})

%% endif
%% endfor

%% endblock body
