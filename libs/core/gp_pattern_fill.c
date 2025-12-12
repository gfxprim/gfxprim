// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <core/gp_pattern_fill.h>

static uint8_t pattern_50[4] = {0xaa, 0x55, 0xaa, 0x55};
static uint8_t pattern_asc_25[4] = {0x88, 0x44, 0x22, 0x11};
static uint8_t pattern_dsc_25[4] = {0x11, 0x22, 0x44, 0x88};
static uint8_t pattern_asc_75[4] = {0x77, 0xbb, 0xdd, 0xee};
static uint8_t pattern_dsc_75[4] = {0xee, 0xdd, 0xbb, 0x77};
static uint8_t pattern_dots_12_5[4] = {0x88, 0x00, 0x00, 0x00};
static uint8_t pattern_dots_25[4] = {0xa0, 0x00, 0xa0, 0x00};
static uint8_t pattern_dots_75[4] = {0x5f, 0xff, 0x5f, 0xff};
static uint8_t pattern_dots_87_5[4] = {0x9f, 0xff, 0x9f, 0xff};

uint8_t *gp_pixel_pattern_lookup_table[GP_PIXEL_PATTERN_CNT] = {
	/* This is not used! */
	[GP_PIXEL_PATTERN_NONE] = NULL,
	[GP_PIXEL_PATTERN_50] = pattern_50,
	[GP_PIXEL_PATTERN_ASC_25] = pattern_asc_25,
	[GP_PIXEL_PATTERN_DSC_25] = pattern_dsc_25,
	[GP_PIXEL_PATTERN_ASC_75] = pattern_asc_75,
	[GP_PIXEL_PATTERN_DSC_75] = pattern_dsc_75,
	[GP_PIXEL_PATTERN_DOTS_12_5] = pattern_dots_12_5,
	[GP_PIXEL_PATTERN_DOTS_25] = pattern_dots_25,
	[GP_PIXEL_PATTERN_DOTS_75] = pattern_dots_75,
	[GP_PIXEL_PATTERN_DOTS_87_5] = pattern_dots_87_5,
};
