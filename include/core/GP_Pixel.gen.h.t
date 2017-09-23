@ include header.t
/*
 * Pixel type definitions and functions.
 * Do not include directly, use GP_Pixel.h
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2013-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * List of all known pixel types
 */
typedef enum GP_PixelType {
@ for pt in pixeltypes:
	GP_PIXEL_{{ pt.name }},
@ end
	GP_PIXEL_MAX,
} GP_PixelType;

@ for pt in pixeltypes:
#define GP_PIXEL_{{ pt.name }} GP_PIXEL_{{ pt.name }}
@ end

@ for pt in pixeltypes:
@     if not pt.is_unknown():
/* Automatically generated code for pixel type {{ pt.name }}
 *
 * Size (bpp): {{ pt.pixelsize.size }} ({{ pt.pixelsize.suffix }})
 * Bit endian: {{ pt.pixelsize.bit_endian_const }}
 * Pixel structure: {{ "".join(pt.bits) }}
 * Channels:
@         for c in pt.chanslist:
 *   {{ c[0] }}  offset:{{ c[1] }} size:{{ c[2] }}
@         end
 */

/*
 * macros to get channels of pixel type {{ pt.name }}
 */
@         for c in pt.chanslist:
#define GP_Pixel_GET_{{ c[0] }}_{{ pt.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))
@         end

/*
 * macros to get address and bit-offset of a pixel {{ pt.name }} in a pixmap
 */
#define GP_PIXEL_ADDR_{{ pt.name }}(pixmap, x, y) GP_PIXEL_ADDR_{{ pt.pixelsize.suffix }}(pixmap, x, y)
#define GP_PIXEL_ADDR_OFFSET_{{ pt.name }}(x) GP_PIXEL_ADDR_OFFSET_{{ pt.pixelsize.suffix }}(x)

/*
 * macros to create GP_Pixel of pixel type {{ pt.name }} directly from given values.
 * The values MUST be already clipped/converted to relevant value ranges.
 */
#define GP_Pixel_CREATE_{{ pt.name }}({{ ', '.join(pt.chan_names) }}) (0\
@         for c in pt.chanslist:
	+ (({{ c[0] }}) << {{ c[1] }}) \
@         end
	)

@ end

/*
 * macros for branching on PixelType (similar to GP_FnPerBpp macros)
 */

@ for r in [('', ''), ('return ', 'RET_')]:
#define GP_FN_{{ r[1] }}PER_PIXELTYPE(FN_NAME, type, ...)\
	switch (type) { \
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		case GP_PIXEL_{{ pt.name }}:\
			{{ r[0] }}FN_NAME{{'##'}}_{{ pt.name }}(__VA_ARGS__);\
@             if not r[0]:
		break;\
@             end
@     end
		default: GP_ABORT("Invalid PixelType %d", type);\
	}

