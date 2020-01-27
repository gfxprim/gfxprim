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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <core/gp_debug.h>
#include <loaders/GP_LineConvert.h>

static void abc888_to_cba888(const uint8_t *inbuf, uint8_t *outbuf,
                             unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		outbuf[2] = inbuf[0];
		outbuf[0] = inbuf[2];
		outbuf[1] = inbuf[1];

		outbuf+=3;
		inbuf+=3;
	}
}

static void xabc8888_to_abc888(const uint8_t *inbuf, uint8_t *outbuf,
                               unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		outbuf[0] = inbuf[0];
		outbuf[1] = inbuf[1];
		outbuf[2] = inbuf[2];

		outbuf+=3;
		inbuf+=4;
	}
}

static void xabc8888_to_cba888(const uint8_t *inbuf, uint8_t *outbuf,
                               unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		outbuf[0] = inbuf[2];
		outbuf[1] = inbuf[1];
		outbuf[2] = inbuf[0];

		outbuf+=3;
		inbuf+=4;
	}
}

gp_line_convert gp_line_convert_get(gp_pixel_type in, gp_pixel_type out)
{
	switch (in) {
	case GP_PIXEL_RGB888:
		switch (out) {
		case GP_PIXEL_BGR888:
			return abc888_to_cba888;
		break;
		default:
		break;
		}
	break;
	case GP_PIXEL_BGR888:
		switch (out) {
		case GP_PIXEL_RGB888:
			return abc888_to_cba888;
		break;
		default:
		break;
		}
	break;
	case GP_PIXEL_xRGB8888:
		switch (out) {
		case GP_PIXEL_RGB888:
			return xabc8888_to_abc888;
		break;
		case GP_PIXEL_BGR888:
			return xabc8888_to_cba888;
		break;
		default:
		break;
		}
	break;
	default:
	break;
	}

	return NULL;
}

gp_pixel_type gp_line_convertible(gp_pixel_type in, gp_pixel_type out[])
{
	unsigned int i;

	GP_DEBUG(1, "Trying to find conversion for %s", gp_pixel_type_name(in));


	for (i = 0; out[i] != GP_PIXEL_UNKNOWN; i++) {
		if (out[i] == in) {
			GP_DEBUG(1, "Found identity for %s",
			         gp_pixel_type_name(in));
			return in;
		}
	}

	for (i = 0; out[i] != GP_PIXEL_UNKNOWN; i++) {
		if (gp_line_convert_get(in, out[i])) {
			GP_DEBUG(1, "Found %s -> %s", gp_pixel_type_name(in),
			         gp_pixel_type_name(out[i]));
			return out[i];
		}
	}

	GP_DEBUG(1, "Not found");

	return GP_PIXEL_UNKNOWN;
}
