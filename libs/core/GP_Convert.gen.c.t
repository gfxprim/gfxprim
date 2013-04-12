/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 * Copyright (C) 2011      Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}Convert PixelType values macros and functions{% endblock %}

%% block body

#include "GP_Convert.h"

{#
 # Loop around pixel types central for the conversion.
-#}
%% for pt in [pixeltypes_dict['RGB888'], pixeltypes_dict['RGBA8888']]

GP_Pixel GP_{{ pt.name }}ToPixel(GP_Pixel pixel, GP_PixelType type)
{
	GP_Pixel p = 0;
	switch (type) {
%% for tf in pixeltypes
%% if tf.is_unknown()
		case GP_PIXEL_UNKNOWN:
			GP_ABORT("Cannot convert {{ pt.name }} to GP_PIXEL_UNKNOWN");
			break;
%% elif tf.is_palette()
		case GP_PIXEL_{{ tf.name }}:
			GP_ABORT("Cannot convert {{ pt.name }} to palette type {{ tf.name }}");
			break;
%% else
		case GP_PIXEL_{{ tf.name }}:
			GP_Pixel_{{ pt.name }}_TO_{{ tf.name }}(pixel, p);
			break;
%% endif
%% endfor
		default:
			GP_ABORT("Unknown PixelType %ud", type);
	}
	return p;
}


GP_Pixel GP_PixelTo{{ pt.name }}(GP_Pixel pixel, GP_PixelType type)
{
	GP_Pixel p = 0;
	switch (type) {
%% for sf in pixeltypes
%% if sf.is_unknown()
		case GP_PIXEL_UNKNOWN:
			GP_ABORT("Cannot convert from GP_PIXEL_UNKNOWN");
			break;
%% elif sf.is_palette()
		case GP_PIXEL_{{ sf.name }}:
			GP_ABORT("Cannot convert from palette type {{ sf.name }} (yet)");
			break;
%% else
		case GP_PIXEL_{{ sf.name }}:
			GP_Pixel_{{ sf.name }}_TO_{{ pt.name }}(pixel, p);
			break;
%% endif
%% endfor
		default:
			GP_ABORT("Unknown PixelType %u", type);
	}
	return p;
}

%% endfor

%% endblock body
