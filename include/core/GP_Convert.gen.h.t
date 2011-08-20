%% extends "base.h.t"

{% block descr %}Convert PixelType values macros and functions{% endblock %}


%% macro GP_Pixel_TYPE_TO_TYPE(pt1, pt2)
/*** {{ pt1.name }} -> {{ pt2.name }} ***
 * macro reads p1 ({{ pt1.name }} at bit-offset o1)
 * and writes to p2 ({{ pt2.name }} at bit-offset o2)
 * the relevant part of p2 is assumed to be cleared (zero) */
#define GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, o1, p2, o2) do { \
%% for c2 in pt2.chanslist
{# case 1: just copy a channel -#}
%% if c2[0] in pt1.chans.keys()
%% set c1 = pt1.chans[c2[0]]
        /* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2,\
                GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1))); \
{# case 2: set A to full opacity (not present in source) -#}
%% elif c2[0]=='A'
        /* A:={{ hex(c2[2]) }} */GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2, {{ hex(c2[2]) }}); \
{# case 3: calculate V as average of RGB -#}
%% elif c2[0]=='V' and pt1.is_rgb()
	/* V:=RGB_avg */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2, ( \
%% for c1 in [pt1.chans['R'], pt1.chans['G'], pt1.chans['B']]
                /* {{c1[0]}} */ GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1)) + \
%% endfor
        0)/3);\
{# case 4: set each RGB to V -#}
%% elif c2[0] in 'RGB' and pt1.is_gray()
%% set c1 = pt1.chans['V']
        /* {{ c2[0] }}:=V */ GP_SET_BITS({{c2[1]}}+o2, {{c2[2]}}, p2,\
                GP_SCALE_VAL_{{c1[2]}}_{{c2[2]}}(GP_GET_BITS({{c1[1]}}+o1, {{c1[2]}}, p1))); \
{# case 5: invalid mapping -#}
%% else
{{ error('Channel conversion ' + pt1.name + ' to ' + pt2.name + ' not supported.') }}
%% endif
%% endfor
} while (0)

/* a version without offsets */
#define GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}(p1, p2) \
        GP_Pixel_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, 0, p2, 0)

%% endmacro


%% block body
#include "GP_Common.h"
#include "GP_Context.h"
#include "GP_Pixel.h"

{#
 # Loop around "central" pixel types
-#}
%% for pt in [pixeltypes_dict['RGB888'], pixeltypes_dict['RGBA8888']]
%% for i in pixeltypes
%% if not i.is_unknown()
%% if not i.is_palette()
{{ GP_Pixel_TYPE_TO_TYPE(pt, i) }}
%% if i.name not in ['RGB888', 'RGBA8888']
{{ GP_Pixel_TYPE_TO_TYPE(i, pt) }}
%% endif
%% endif
%% endif
%% endfor

/* 
 * Convert {{ pt.name }} to any other PixelType
 * Does not work on palette types at all (yet)
 */
GP_Pixel GP_{{ pt.name }}ToPixel(GP_Pixel pixel, GP_PixelType type);

/* 
 * Function converting to {{ pt.name }} from any other PixelType
 * Does not work on palette types at all (yet)
 */
GP_Pixel GP_PixelTo{{ pt.name }}(GP_Pixel pixel, GP_PixelType type);

%% endfor

/* Experimental macros testing generated scripts */
{{ GP_Pixel_TYPE_TO_TYPE(pixeltypes_dict['RGB565'], pixeltypes_dict['RGBA8888']) }}
{{ GP_Pixel_TYPE_TO_TYPE(pixeltypes_dict['RGBA8888'], pixeltypes_dict['G2']) }}
{{ GP_Pixel_TYPE_TO_TYPE(pixeltypes_dict['VA12'], pixeltypes_dict['RGBA8888']) }}

%% endblock body
