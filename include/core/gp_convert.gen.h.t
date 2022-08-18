@ include header.t
/*
 * Convert PixelType values macros and functions
 *
 * Copyright (C) 2011-2014 Cyril Hrubis <metan@ucw.cz>
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 */
@
@ # RGB -> CMYK requires special handling
@ def rgb_to_cmyk(in_pix, out_pix):
@     R = in_pix.chans['R']
@     G = in_pix.chans['G']
@     B = in_pix.chans['B']
@     C = out_pix.chans['C']
@     M = out_pix.chans['M']
@     Y = out_pix.chans['Y']
@     K = out_pix.chans['K']
@     max_size = max(R.size, G.size, B.size)
@     max_val = 2 ** max_size - 1
	gp_pixel gp_r = GP_SCALE_VAL_{{ R.size }}_{{ max_size }}(GP_GET_BITS({{ R.off }}+o1, {{ R.size }}, p1)); \
	gp_pixel gp_g = GP_SCALE_VAL_{{ G.size }}_{{ max_size }}(GP_GET_BITS({{ G.off }}+o1, {{ G.size }}, p1)); \
	gp_pixel gp_b = GP_SCALE_VAL_{{ B.size }}_{{ max_size }}(GP_GET_BITS({{ B.off }}+o1, {{ B.size }}, p1)); \
	gp_pixel gp_k = GP_MAX3(gp_r, gp_g, gp_b); \
	GP_SET_BITS({{ C.off }}+o2, {{ C.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ C.size }}((gp_k - gp_r))); \
	GP_SET_BITS({{ M.off }}+o2, {{ M.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ M.size }}((gp_k - gp_g))); \
	GP_SET_BITS({{ Y.off }}+o2, {{ Y.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ Y.size }}((gp_k - gp_b))); \
	GP_SET_BITS({{ K.off }}+o2, {{ K.size }}, p2, GP_SCALE_VAL_{{ max_size }}_{{ K.size }}({{ max_val }} - gp_k)); \
@ end
@
@ def pixel_type_to_type(pt1, pt2):
/*** {{ pt1.name }} -> {{ pt2.name }} ***
 * macro reads p1 ({{ pt1.name }} at bit-offset o1)
 * and writes to p2 ({{ pt2.name }} at bit-offset o2)
 * the relevant part of p2 is assumed to be cleared (zero) */
#define GP_PIXEL_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, o1, p2, o2) do { \
@     # special cases
@     if pt1.is_rgb() and pt2.is_cmyk():
@         rgb_to_cmyk(pt1, pt2)
@     else:
@         for c2 in pt2.chanslist:
@             # case 1: just copy a channel
@             if c2[0] in pt1.chans.keys():
@                 c1 = pt1.chans[c2[0]]
        /* {{ c2[0] }}:={{ c1[0] }} */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1))); \
@             # case 2: set A to full opacity (not present in source)
@             elif c2[0]=='A':
        /* A:={{ c2.C_max }} */GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2, {{ c2.C_max }}); \
@             # case 3: calculate V as average of RGB
@             elif c2[0]=='V' and pt1.is_rgb():
	/* V:=RGB_avg */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2, ( \
@                 for c1 in [pt1.chans['R'], pt1.chans['G'], pt1.chans['B']]:
                /* {{ c1.name }} */ GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1)) + \
@                 end
        0)/3);\
@             # case 4: set each RGB to V -#}
@             elif c2[0] in 'RGB' and pt1.is_gray():
@                 c1 = pt1.chans['V']
        /* {{ c2[0] }}:=V */ GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                GP_SCALE_VAL_{{ c1.size }}_{{ c2.size }}(GP_GET_BITS({{ c1.off }}+o1, {{ c1.size }}, p1))); \
@             # case 5: CMYK to RGB
@             elif c2[0] in 'RGB' and pt1.is_cmyk():
@                 K = pt1.chans['K']
@                 if c2[0] == 'R':
@                     V = pt1.chans['C']
@                 elif c2[0] == 'G':
@                     V = pt1.chans['M']
@                 else:
@                     V = pt1.chans['Y']
@                 end
	GP_SET_BITS({{ c2.off }}+o2, {{ c2.size }}, p2,\
                    (({{ c2.C_max }} * ({{ K.C_max }} - GP_GET_BITS({{ K.off }}+o1, {{ K.size }}, p1)) * \
                     ({{ V.C_max }} - GP_GET_BITS({{ V.off }}+o1, {{ V.size }}, p1)))) / ({{ K.C_max }} * {{ V.C_max }})); \
@             # case 7: invalid mapping
@             else:
{{ error('Channel conversion ' + pt1.name + ' to ' + pt2.name + ' not supported.') }}
@     end
} while (0)

/* a version without offsets */
#define GP_PIXEL_{{ pt1.name }}_TO_{{ pt2.name }}(p1, p2) \
        GP_PIXEL_{{ pt1.name }}_TO_{{ pt2.name }}_OFFSET(p1, 0, p2, 0)

@ end

#include <core/gp_get_set_bits.h>
#include "core/gp_pixmap.h"
#include <core/gp_pixel.h>

@
@ # Loop around "central" pixel types
@
@ for pt in [pixeltypes_dict['RGB888'], pixeltypes_dict['RGBA8888']]:
@     for i in pixeltypes:
@         if not i.is_unknown() and not i.is_palette():
@             pixel_type_to_type(pt, i)
@             if i.name not in ['RGB888', 'RGBA8888']:
@                 pixel_type_to_type(i, pt)
@     end

/*
 * Convert {{ pt.name }} to any other PixelType
 * Does not work on palette types at all (yet)
 */
gp_pixel gp_{{ pt.name }}_to_pixel(gp_pixel pixel, gp_pixel_type type);

/*
 * Function converting to {{ pt.name }} from any other PixelType
 * Does not work on palette types at all (yet)
 */
gp_pixel gp_pixel_to{{ pt.name }}(gp_pixel pixel, gp_pixel_type type);

@ end

/* Experimental macros testing generated scripts */
@ pixel_type_to_type(pixeltypes_dict['RGB565'], pixeltypes_dict['RGBA8888'])
@ pixel_type_to_type(pixeltypes_dict['RGBA8888'], pixeltypes_dict['G2'])
