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

#include "core/GP_Debug.h"
#include "GP_LineConvert.h"

static void ABC888_to_CBA888(const uint8_t *inbuf, uint8_t *outbuf,
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

static void xABC8888_to_ABC888(const uint8_t *inbuf, uint8_t *outbuf,
                               unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		outbuf[0] = inbuf[1];
		outbuf[1] = inbuf[2];
		outbuf[2] = inbuf[3];

		outbuf+=3;
		inbuf+=4;
	}
}

static void xABC8888_to_CBA888(const uint8_t *inbuf, uint8_t *outbuf,
                               unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		outbuf[0] = inbuf[3];
		outbuf[1] = inbuf[2];
		outbuf[2] = inbuf[1];

		outbuf+=3;
		inbuf+=4;
	}
}

GP_LineConvert GP_LineConvertGet(GP_PixelType in, GP_PixelType out)
{
	switch (in) {
	case GP_PIXEL_RGB888:
		switch (out) {
		case GP_PIXEL_BGR888:
			return ABC888_to_CBA888;
		break;
		default:
		break;
		}
	break;
	case GP_PIXEL_BGR888:
		switch (out) {
		case GP_PIXEL_RGB888:
			return ABC888_to_CBA888;
		break;
		default:
		break;
		}
	break;
	case GP_PIXEL_xRGB8888:
		switch (out) {
		case GP_PIXEL_RGB888:
			return xABC8888_to_ABC888;
		break;
		case GP_PIXEL_BGR888:
			return xABC8888_to_CBA888;
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

GP_PixelType GP_LineConvertible(GP_PixelType in, GP_PixelType out[])
{
	unsigned int i;

	GP_DEBUG(1, "Trying to find conversion for %s", GP_PixelTypeName(in));


	for (i = 0; out[i] != GP_PIXEL_UNKNOWN; i++) {
		if (out[i] == in) {
			GP_DEBUG(1, "Found identity for %s", GP_PixelTypeName(in));
			return in;
		}
	}

	for (i = 0; out[i] != GP_PIXEL_UNKNOWN; i++) {
		if (GP_LineConvertGet(in, out[i])) {
			GP_DEBUG(1, "Found %s -> %s", GP_PixelTypeName(in),
			         GP_PixelTypeName(out[i]));
			return out[i];
		}
	}

	GP_DEBUG(1, "Not found");

	return GP_PIXEL_UNKNOWN;
}
