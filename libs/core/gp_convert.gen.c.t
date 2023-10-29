@ include source.t
/*
 * Convert PixelType values macros and functions
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_convert.h>
@
@ # Loop around pixel types central for the conversion.
@ for pt in [pixeltypes_dict['RGB888'], pixeltypes_dict['RGBA8888'], pixeltypes_dict['G8']]:

gp_pixel gp_{{ pt.name }}_to_pixel(gp_pixel pixel, gp_pixel_type type)
{
	gp_pixel p = 0;
	switch (type) {
@     for tf in pixeltypes:
@         if tf.is_unknown():
		case GP_PIXEL_UNKNOWN:
			GP_ABORT("Cannot convert {{ pt.name }} to GP_PIXEL_UNKNOWN");
			break;
@         elif tf.is_palette():
		case GP_PIXEL_{{ tf.name }}:
			GP_ABORT("Cannot convert {{ pt.name }} to palette type {{ tf.name }}");
			break;
@         else:
		case GP_PIXEL_{{ tf.name }}:
			GP_PIXEL_{{ pt.name }}_TO_{{ tf.name }}(pixel, p);
			break;
@     end
		default:
			GP_ABORT("Unknown PixelType %ud", type);
	}
	return p;
}

gp_pixel gp_pixel_to_{{ pt.name }}(gp_pixel pixel, gp_pixel_type type)
{
	gp_pixel p = 0;
	switch (type) {
@     for sf in pixeltypes:
@         if sf.is_unknown():
		case GP_PIXEL_UNKNOWN:
			GP_ABORT("Cannot convert from GP_PIXEL_UNKNOWN");
			break;
@         elif sf.is_palette():
		case GP_PIXEL_{{ sf.name }}:
			GP_ABORT("Cannot convert from palette type {{ sf.name }} (yet)");
			break;
@         else:
		case GP_PIXEL_{{ sf.name }}:
			GP_PIXEL_{{ sf.name }}_TO_{{ pt.name }}(pixel, p);
			break;
@     end
		default:
			GP_ABORT("Unknown PixelType %u", type);
	}
	return p;
}

