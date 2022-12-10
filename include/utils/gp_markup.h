// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef UTILS_GP_MARKUP_H
#define UTILS_GP_MARKUP_H

#include <stdint.h>
#include <stddef.h>

enum gp_markup_flags {
	GP_MARKUP_BOLD = 0x01,
	GP_MARKUP_LARGE = 0x02,
	GP_MARKUP_MONO = 0x04,
	GP_MARKUP_SUB = 0x08,
	GP_MARKUP_SUP = 0x10,
	GP_MARKUP_UNDERLINE = 0x20,
	GP_MARKUP_STRIKE = 0x40,
};

enum gp_markup_color {
	GP_MARKUP_DEFAULT = 0x00,
	/* 16 colors */
	GP_MARKUP_RED,
	GP_MARKUP_GREEN,
	GP_MARKUP_YELLOW,
	GP_MARKUP_BLUE,
	GP_MARKUP_MAGENTA,
	GP_MARKUP_CYAN,
	GP_MARKUP_GRAY,
	GP_MARKUP_BR_RED,
	GP_MARKUP_BR_GREEN,
	GP_MARKUP_BR_YELLOW,
	GP_MARKUP_BR_BLUE,
	GP_MARKUP_BR_MAGENTA,
	GP_MARKUP_BR_CYAN,
	GP_MARKUP_WHITE,
};

typedef struct gp_markup_glyph {
	/* unicode glyph */
	uint32_t glyph;
	/* text format, bold, monospace, .... */
	uint16_t fmt;
	union {
		uint16_t fg_color;
		int16_t space_pad;
	};
} gp_markup_glyph;

typedef struct gp_markup {
	uint32_t glyph_cnt;
	gp_markup_glyph glyphs[];
} gp_markup;

void gp_markup_dump(gp_markup *self);

#endif /* UTILS_GP_MARKUP_H */
