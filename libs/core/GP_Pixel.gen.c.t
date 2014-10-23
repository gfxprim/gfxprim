@ include source.t
/*
 * Pixel type definitions and functions
 *
 * Copyright (C) 2011-2012 Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include "GP_Pixel.h"
#include "GP_GetSetBits.h"
@
@ def getflags(pt):
@     flags = []
@     if pt.is_alpha():
@         flags.append('GP_PIXEL_HAS_ALPHA')
@     if pt.is_rgb():
@         flags.append('GP_PIXEL_IS_RGB')
@     if pt.is_palette():
@         flags.append('GP_PIXEL_IS_PALETTE')
@     if pt.is_gray():
@         flags.append('GP_PIXEL_IS_GRAYSCALE')
@     if pt.is_cmyk():
@         flags.append('GP_PIXEL_IS_CMYK')
@     if flags:
@         return ' | '.join(flags)
@     else:
@         return 0
@ end

/*
 * Description of all known pixel types
 */
const GP_PixelTypeDescription GP_PixelTypes [GP_PIXEL_MAX] = {
@ for pt in pixeltypes:
	/* GP_PIXEL_{{ pt.name }} */ {
		.type	= GP_PIXEL_{{ pt.name }},
		.name	= "{{ pt.name }}",
		.size	= {{ pt.pixelsize.size }},
		.bit_endian  = {{ pt.pixelsize.bit_endian_const }},
		.numchannels = {{ len(pt.chanslist) }},
		.bitmap      = "{{ ''.join(pt.bits) }}",
		.flags       = {{ getflags(pt) }},
		.channels    = {
@     for c in pt.chanslist:
			{ .name = "{{ c[0] }}", .offset = {{ c[1] }}, .size = {{ c[2] }} },
@     end
		}
	},
@ end
};

@ for pt in pixeltypes:
@     if not pt.is_unknown():
/*
 * snprintf a human readable value of pixel type {{pt.name}}
 */
static void GP_PixelSNPrint_{{ pt.name }}(char *buf, size_t len, GP_Pixel p)
{
	snprintf(buf, len, "{{ pt.name }} 0x%0{{ (pt.pixelsize.size+3)//4 }}x {{ '=%d '.join(pt.chan_names) + '=%d' }}",
		GP_GET_BITS(0, {{ pt.pixelsize.size }}, p),
		{{ arr_to_params(pt.chan_names, 'GP_Pixel_GET_', '_' + pt.name + '(p)') }});
}

@ end

void GP_PixelSNPrint(char *buf, size_t len, GP_Pixel p, GP_PixelType pixel_type)
{
	GP_FN_PER_PIXELTYPE(GP_PixelSNPrint, pixel_type, buf, len, p);
}

void GP_PixelPrint(GP_Pixel p, GP_PixelType pixel_type)
{
	char buf[256];
	GP_PixelSNPrint(buf, sizeof(buf), p, pixel_type);
	puts(buf);
}
