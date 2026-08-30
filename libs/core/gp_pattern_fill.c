// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <core/gp_pixel.h>
#include <core/gp_pattern_fill.h>

/*
 * The pattern tiles, four pixels by four.
 *
 * Each row is read as "lookup[y % 4] & (8 >> (x % 4))", i.e. the leftmost
 * pixel of the tile is the highest bit of the nibble, and the low nibble is
 * duplicated into the high one so that a byte covers eight pixels.
 *
 * This is the only copy of them. Everything that draws a pattern looks them up
 * through gp_pixel_pattern_lookup_table_get(), which is what keeps a row, a
 * column and a fill drawing the same thing.
 *
 * No pattern means that the channel takes the foreground color everywhere, so
 * GP_PIXEL_PATTERN_FG is a tile of all ones rather than a hole in the table
 * that every lookup has to test for.
 */
static uint8_t pattern_fg[4] = {0xff, 0xff, 0xff, 0xff};
static uint8_t pattern_50[4] = {0x55, 0xaa, 0x55, 0xaa};
static uint8_t pattern_asc_25[4] = {0x11, 0x22, 0x44, 0x88};
static uint8_t pattern_dsc_25[4] = {0x88, 0x44, 0x22, 0x11};
static uint8_t pattern_asc_75[4] = {0xee, 0xdd, 0xbb, 0x77};
static uint8_t pattern_dsc_75[4] = {0x77, 0xbb, 0xdd, 0xee};
static uint8_t pattern_dots_6_25[4] = {0x88, 0x00, 0x00, 0x00};
static uint8_t pattern_dots_12_5[4] = {0x88, 0x00, 0x22, 0x00};
static uint8_t pattern_dots_25[4] = {0xaa, 0x00, 0xaa, 0x00};
static uint8_t pattern_dots_75[4] = {0x55, 0xff, 0x55, 0xff};
static uint8_t pattern_dots_87_5[4] = {0x77, 0xff, 0xdd, 0xff};
static uint8_t pattern_dots_93_75[4] = {0x77, 0xff, 0xff, 0xff};

uint8_t *gp_pixel_pattern_lookup_table[GP_PIXEL_PATTERN_CNT] = {
	[GP_PIXEL_PATTERN_FG] = pattern_fg,
	[GP_PIXEL_PATTERN_50] = pattern_50,
	[GP_PIXEL_PATTERN_ASC_25] = pattern_asc_25,
	[GP_PIXEL_PATTERN_DSC_25] = pattern_dsc_25,
	[GP_PIXEL_PATTERN_ASC_75] = pattern_asc_75,
	[GP_PIXEL_PATTERN_DSC_75] = pattern_dsc_75,
	[GP_PIXEL_PATTERN_DOTS_6_25] = pattern_dots_6_25,
	[GP_PIXEL_PATTERN_DOTS_12_5] = pattern_dots_12_5,
	[GP_PIXEL_PATTERN_DOTS_25] = pattern_dots_25,
	[GP_PIXEL_PATTERN_DOTS_75] = pattern_dots_75,
	[GP_PIXEL_PATTERN_DOTS_87_5] = pattern_dots_87_5,
	[GP_PIXEL_PATTERN_DOTS_93_75] = pattern_dots_93_75,
};

enum gp_pixel_pattern gp_pixel_pattern_by_shade(uint8_t density)
{
	if (density < 16)
		return GP_PIXEL_PATTERN_BG;

	if (density < 32)
		return GP_PIXEL_PATTERN_DOTS_6_25;

	if (density < 48)
		return GP_PIXEL_PATTERN_DOTS_12_5;

	if (density < 96)
		return GP_PIXEL_PATTERN_DOTS_25;

	if (density < 160)
		return GP_PIXEL_PATTERN_50;

	if (density < 208)
		return GP_PIXEL_PATTERN_DOTS_75;

	if (density < 240)
		return GP_PIXEL_PATTERN_DOTS_87_5;

	if (density < 248)
		return GP_PIXEL_PATTERN_DOTS_93_75;

	return GP_PIXEL_PATTERN_FG;
}
