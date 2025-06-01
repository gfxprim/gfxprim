// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <core/gp_bit_swap.h>
#include <loaders/gp_line_convert.h>

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

static void bitswap_1bpp(const uint8_t *inbuf, uint8_t *outbuf, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < (len+7)/8; i++)
		outbuf[i] = GP_BIT_SWAP_B1(inbuf[i]);
}

static void bitswap_2bpp(const uint8_t *inbuf, uint8_t *outbuf, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < (len+3)/4; i++)
		outbuf[i] = GP_BIT_SWAP_B2(inbuf[i]);
}

static void bitswap_4bpp(const uint8_t *inbuf, uint8_t *outbuf, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < (len+1)/2; i++)
		outbuf[i] = GP_BIT_SWAP_B4(inbuf[i]);
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
	case GP_PIXEL_G1_UB:
		if (out == GP_PIXEL_G1_DB)
			return bitswap_1bpp;
	break;
	case GP_PIXEL_G1_DB:
		if (out == GP_PIXEL_G1_UB)
			return bitswap_1bpp;
	break;
	case GP_PIXEL_G2_UB:
		if (out == GP_PIXEL_G2_DB)
			return bitswap_2bpp;
	break;
	case GP_PIXEL_G2_DB:
		if (out == GP_PIXEL_G2_UB)
			return bitswap_2bpp;
	break;
	case GP_PIXEL_G4_UB:
		if (out == GP_PIXEL_G4_DB)
			return bitswap_4bpp;
	break;
	case GP_PIXEL_G4_DB:
		if (out == GP_PIXEL_G4_UB)
			return bitswap_4bpp;
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
