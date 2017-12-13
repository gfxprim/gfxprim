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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <text/GP_Font.h>

static int8_t default_console_glyphs[] = {
	/* ' ' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '!' */	7, 11, 0, 9, 8,
			0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
	/* '"' */	7, 11, 0, 9, 8,
			0x24, 0x24, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '#' */	7, 11, 0, 9, 8,
			0x00, 0x14, 0x14, 0x7e, 0x28, 0x28, 0xfc, 0x50, 0x50, 0x00, 0x00,
	/* '$' */	7, 11, 0, 9, 8,
			0x10, 0x7c, 0x92, 0x90, 0x7c, 0x12, 0x92, 0x7c, 0x10, 0x00, 0x00,
	/* '%' */	7, 11, 0, 9, 8,
			0x61, 0x92, 0x94, 0x68, 0x10, 0x2c, 0x52, 0x92, 0x0c, 0x00, 0x00,
	/* '&' */	7, 11, 0, 9, 8,
			0x30, 0x48, 0x48, 0x30, 0x56, 0x88, 0x88, 0x88, 0x76, 0x00, 0x00,
	/* ''' */	7, 11, 0, 9, 8,
			0x0c, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '(' */	7, 11, 0, 9, 8,
			0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x08, 0x00, 0x00,
	/* ')' */	7, 11, 0, 9, 8,
			0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x00, 0x00,
	/* '*' */	7, 11, 0, 9, 8,
			0x00, 0x10, 0x54, 0x54, 0x38, 0x54, 0x54, 0x10, 0x00, 0x00, 0x00,
	/* '+' */	7, 11, 0, 9, 8,
			0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
	/* ',' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x10, 0x00,
	/* '-' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '.' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
	/* '/' */	7, 11, 0, 9, 8,
			0x00, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x00, 0x00,
	/* '0' */	7, 11, 1, 9, 8,
			0x78, 0x84, 0x8c, 0x94, 0xb4, 0xa4, 0xc4, 0x84, 0x78, 0x00, 0x00,
	/* '1' */	7, 11, 1, 9, 8,
			0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00,
	/* '2' */	7, 11, 1, 9, 8,
			0x78, 0x84, 0x84, 0x04, 0x18, 0x60, 0x80, 0x80, 0xfc, 0x00, 0x00,
	/* '3' */	7, 11, 1, 9, 8,
			0x78, 0x84, 0x84, 0x04, 0x18, 0x04, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '4' */	7, 11, 1, 9, 8,
			0x18, 0x28, 0x48, 0x48, 0x88, 0xfc, 0x08, 0x08, 0x08, 0x00, 0x00,
	/* '5' */	7, 11, 1, 9, 8,
			0xfc, 0x80, 0x80, 0xf8, 0x04, 0x04, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '6' */	7, 11, 1, 9, 8,
			0x78, 0x80, 0x80, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '7' */	7, 11, 1, 9, 8,
			0xfc, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* '8' */	7, 11, 1, 9, 8,
			0x78, 0x84, 0x84, 0x84, 0x78, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '9' */	7, 11, 1, 9, 8,
	                0x78, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x04, 0x04, 0x78, 0x00, 0x00,
	/* ':' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00,
	/* ';' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x08, 0x10, 0x00,
	/* '<' */	7, 11, 0, 9, 8,
			0x00, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00,
	/* '=' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
	/* '>' */	7, 11, 0, 9, 8,
			0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00, 0x00,
	/* '?' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x02, 0x0c, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00,
	/* '@' */	8, 11, 0, 9, 8,
			0x3e, 0x41, 0x9d, 0xa5, 0xa5, 0xa5, 0x9e, 0x41, 0x3e, 0x00, 0x00,
	/* 'A' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'B' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0xfc, 0x82, 0x82, 0x82, 0xfc, 0x00, 0x00,
	/* 'C' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00, 0x00,
	/* 'D' */	7, 11, 0, 9, 8,
			0xf8, 0x84, 0x82, 0x82, 0x82, 0x82, 0x82, 0x84, 0xf8, 0x00, 0x00,
	/* 'E' */	7, 11, 0, 9, 8,
			0xfe, 0x80, 0x80, 0x80, 0xfc, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00,
	/* 'F' */	7, 11, 0, 9, 8,
			0xfe, 0x80, 0x80, 0x80, 0xf8, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 'G' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x80, 0x9e, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'H' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'I' */	7, 11, 0, 9, 8,
			0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00,
	/* 'J' */	7, 11, 0, 9, 8,
			0x7e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00,
	/* 'K' */	7, 11, 0, 9, 8,
			0x82, 0x84, 0x88, 0x90, 0xe0, 0x90, 0x88, 0x84, 0x82, 0x00, 0x00,
	/* 'L' */	7, 11, 0, 9, 8,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00,
	/* 'M' */	7, 11, 0, 9, 8,
			0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'N' */	7, 11, 0, 9, 8,
			0x82, 0xc2, 0xa2, 0xa2, 0x92, 0x8a, 0x8a, 0x86, 0x82, 0x00, 0x00,
	/* 'O' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'P' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 'Q' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x9a, 0x84, 0x7a, 0x00, 0x00,
	/* 'R' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00, 0x00,
	/* 'S' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x7c, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00,
	/* 'T' */	7, 11, 0, 9, 8,
			0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* 'U' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'V' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00,
	/* 'W' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00,
	/* 'X' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x82, 0x00, 0x00,
	/* 'Y' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* 'Z' */	7, 11, 0, 9, 8,
			0xfe, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xfe, 0x00, 0x00,
	/* '[' */	7, 11, 0, 9, 8,
			0x3c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x00, 0x00,
	/* '\' */	7, 11, 0, 9, 8,
			0x00, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x00, 0x00,
	/* ']' */	7, 11, 0, 9, 8,
			0x3c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x3c, 0x00, 0x00,
	/* '^' */	7, 11, 0, 9, 8,
			0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '_' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
	/* '`' */	7, 11, 0, 9, 8,
			0x30, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 'a' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x3c, 0x02, 0x02, 0x3e, 0x42, 0x42, 0x3e, 0x00, 0x00,
	/* 'b' */	7, 11, 0, 9, 8,
			0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x00, 0x00,
	/* 'c' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x3c, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00,
	/* 'd' */	7, 11, 0, 9, 8,
			0x02, 0x02, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00,
	/* 'e' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x3e, 0x00, 0x00,
	/* 'f' */	7, 11, 0, 9, 8,
			0x1e, 0x20, 0x20, 0xfc, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	/* 'g' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c,
	/* 'h' */	7, 11, 0, 9, 8,
	                0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00,
	/* 'i' */	7, 11, 0, 9, 8,
			0x08, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3e, 0x00, 0x00,
	/* 'j' */	7, 11, 0, 9, 8,
			0x08, 0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x70,
	/* 'k' */	7, 11, 0, 9, 8,
			0x40, 0x40, 0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00, 0x00,
	/* 'l' */	7, 11, 0, 9, 8,
			0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0e, 0x00, 0x00,
	/* 'm' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0xfc, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00, 0x00,
	/* 'n' */	7, 11, 0, 9, 8,
	                0x00, 0x00, 0x5c, 0x62, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00,
	/* 'o' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00,
	/* 'p' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x40, 0x40,
	/* 'q' */       7, 11, 0, 9, 8,
			0x00, 0x00, 0x3e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02,
	/* 'r' */       7, 11, 0, 9, 8,
			0x00, 0x00, 0x5e, 0x60, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00,
	/* 's' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x7c, 0x80, 0x80, 0x7c, 0x02, 0x02, 0x7c, 0x00, 0x00,
	/* 't' */       7, 11, 0, 9, 8,
			0x00, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0e, 0x00, 0x00,
	/* 'u' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x00, 0x00,
	/* 'v' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x18, 0x00, 0x00,
	/* 'w' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00, 0x00,
	/* 'x' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x00, 0x00,
	/* 'y' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x3c,
	/* 'z' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x7e, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7e, 0x00, 0x00,
	/* '{' */	7, 11, 0, 9, 8,
			0x0c, 0x10, 0x10, 0x10, 0x60, 0x10, 0x10, 0x10, 0x0c, 0x00, 0x00,
	/* '|' */	7, 11, 0, 9, 8,
			0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	/* '}' */	7, 11, 0, 9, 8,
			0x30, 0x08, 0x08, 0x08, 0x06, 0x08, 0x08, 0x08, 0x30, 0x00, 0x00,
	/* '~' */	7, 11, 0, 9, 8,
			0x32, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const struct gp_font_face gp_default_font = {
	.family_name = "Gfxprim",
	.style_name = "Mono",
	.charset = GP_CHARSET_7BIT,
	.ascend  = 9,
	.descend = 2,
	.max_glyph_width = 8,
	.max_glyph_advance = 8,
	.glyph_bitmap_format = GP_FONT_BITMAP_1BPP,
	.glyphs = default_console_glyphs,
	.glyph_offsets = {16},
};

const gp_font_face *const gp_font_gfxprim_mono = &gp_default_font;

static uint8_t default_proportional_glyphs[] = {
	/* ' ' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '!' */	4, 11, 0, 9, 6,
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20, 0x00, 0x00,
	/* '"' */	8, 11, 0, 9, 9,
			0x24, 0x24, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '#' */	8, 11, 0, 9, 9,
			0x00, 0x12, 0x12, 0x7f, 0x24, 0x24, 0xfe, 0x48, 0x48, 0x00, 0x00,
	/* '$' */	7, 11, 0, 9, 8,
			0x10, 0x7c, 0x92, 0x90, 0x7c, 0x12, 0x92, 0x7c, 0x10, 0x00, 0x00,
	/* '%' */	7, 11, 0, 9, 8,
			0x61, 0x92, 0x94, 0x68, 0x10, 0x2c, 0x52, 0x92, 0x0c, 0x00, 0x00,
	/* '&' */	7, 11, 0, 9, 8,
			0x30, 0x48, 0x48, 0x30, 0x56, 0x88, 0x88, 0x88, 0x76, 0x00, 0x00,
	/* ''' */	4, 11, 0, 9, 5,
			0x30, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '(' */	2, 11, 0, 9, 4,
			0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x00,
	/* ')' */	2, 11, 0, 9, 4,
			0x80, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x00,
	/* '*' */	5, 11, 0, 9, 6,
			0x00, 0x20, 0xa8, 0xa8, 0x70, 0xa8, 0xa8, 0x20, 0x00, 0x00, 0x00,
	/* '+' */	7, 11, 0, 9, 8,
			0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
	/* ',' */	4, 11, 0, 9, 6,
	                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40, 0x00,
	/* '-' */	4, 11, 0, 9, 6,
			0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '.' */	4, 11, 0, 9, 6,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00,
	/* '/' */	5, 11, 0, 9, 6,
			0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 0x40, 0x80, 0x80, 0x00, 0x00,
	/* '0' */	6, 11, 0, 9, 7,
			0x78, 0x84, 0x8c, 0x94, 0xb4, 0xa4, 0xc4, 0x84, 0x78, 0x00, 0x00,
	/* '1' */	6, 11, 0, 9, 7,
			0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xf8, 0x00, 0x00,
	/* '2' */	6, 11, 0, 9, 7,
			0x78, 0x84, 0x84, 0x04, 0x18, 0x60, 0x80, 0x80, 0xfc, 0x00, 0x00,
	/* '3' */	6, 11, 0, 9, 7,
			0x78, 0x84, 0x84, 0x04, 0x18, 0x04, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '4' */	6, 11, 0, 9, 7,
			0x18, 0x28, 0x48, 0x48, 0x88, 0xfc, 0x08, 0x08, 0x08, 0x00, 0x00,
	/* '5' */	6, 11, 0, 9, 7,
			0xfc, 0x80, 0x80, 0xf8, 0x04, 0x04, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '6' */	6, 11, 0, 9, 7,
			0x78, 0x80, 0x80, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '7' */	6, 11, 0, 9, 7,
			0xfc, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* '8' */	6, 11, 0, 9, 7,
			0x78, 0x84, 0x84, 0x84, 0x78, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* '9' */	6, 11, 0, 9, 7,
			0x78, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x04, 0x04, 0x78, 0x00, 0x00,
	/* ':' */	4, 11, 0, 9, 5,
			0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00,
	/* ';' */	4, 11, 0, 9, 5,
			0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x20, 0x40, 0x00,
	/* '<' */	4, 11, 0, 9, 6,
			0x00, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00,
	/* '=' */	5, 11, 0, 9, 6,
			0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00,
	/* '>' */	4, 11, 0, 9, 6,
			0x00, 0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00,
	/* '?' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x02, 0x0c, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00,
	/* '@' */	8, 11, 0, 9, 9,
			0x3e, 0x41, 0x9d, 0xa5, 0xa5, 0xa5, 0x9e, 0x41, 0x3e, 0x00, 0x00,
	/* 'A' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'B' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0xfc, 0x82, 0x82, 0x82, 0xfc, 0x00, 0x00,
	/* 'C' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00, 0x00,
	/* 'D' */	7, 11, 0, 9, 8,
			0xf8, 0x84, 0x82, 0x82, 0x82, 0x82, 0x82, 0x84, 0xf8, 0x00, 0x00,
	/* 'E' */	7, 11, 0, 9, 8,
			0xfe, 0x80, 0x80, 0x80, 0xfc, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00,
	/* 'F' */	7, 11, 0, 9, 8,
			0xfe, 0x80, 0x80, 0x80, 0xf8, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 'G' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x80, 0x9e, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'H' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'I' */	1, 11, 0, 9, 2,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 'J' */	7, 11, 0, 9, 8,
			0x7e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00,
	/* 'K' */	7, 11, 0, 9, 8,
			0x82, 0x84, 0x88, 0x90, 0xe0, 0x90, 0x88, 0x84, 0x82, 0x00, 0x00,
	/* 'L' */	6, 11, 0, 9, 7,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfc, 0x00, 0x00,
	/* 'M' */	7, 11, 0, 9, 8,
			0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00,
	/* 'N' */	7, 11, 0, 9, 8,
			0x82, 0xc2, 0xa2, 0xa2, 0x92, 0x8a, 0x8a, 0x86, 0x82, 0x00, 0x00,
	/* 'O' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'P' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 'Q' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x9a, 0x84, 0x7a, 0x00, 0x00,
	/* 'R' */	7, 11, 0, 9, 8,
			0xfc, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00, 0x00,
	/* 'S' */	7, 11, 0, 9, 8,
			0x7c, 0x82, 0x80, 0x80, 0x7c, 0x02, 0x02, 0x82, 0x7c, 0x00, 0x00,
	/* 'T' */	7, 11, 0, 9, 8,
			0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* 'U' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00,
	/* 'V' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00,
	/* 'W' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x82, 0x82, 0x92, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00,
	/* 'X' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x82, 0x00, 0x00,
	/* 'Y' */	7, 11, 0, 9, 8,
			0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00,
	/* 'Z' */	7, 11, 0, 9, 8,
			0xfe, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xfe, 0x00, 0x00,
	/* '[' */	3, 11, 0, 9, 4,
			0xe0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xe0, 0x00, 0x00,
	/* '\' */	5, 11, 0, 9, 6,
			0x80, 0x80, 0x40, 0x40, 0x20, 0x10, 0x10, 0x08, 0x08, 0x00, 0x00,
	/* ']' */	3, 11, 0, 9, 4,
			0xe0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x00, 0x00,
	/* '^' */	8, 11, 0, 9, 9,
			0x08, 0x14, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '_' */	8, 11, 0, 9, 9,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
	/* '`' */	2, 11, 0, 9, 3,
			0xc0, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 'a' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x78, 0x04, 0x04, 0x7c, 0x84, 0x84, 0x7c, 0x00, 0x00,
	/* 'b' */	6, 11, 0, 9, 7,
			0x80, 0x80, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x84, 0xf8, 0x00, 0x00,
	/* 'c' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x78, 0x84, 0x80, 0x80, 0x80, 0x84, 0x78, 0x00, 0x00,
	/* 'd' */	6, 11, 0, 9, 7,
			0x04, 0x04, 0x7c, 0x84, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x00, 0x00,
	/* 'e' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x78, 0x84, 0x84, 0xfc, 0x80, 0x80, 0x7c, 0x00, 0x00,
	/* 'f' */	6, 11, 0, 9, 7,
			0x1e, 0x20, 0x20, 0xfc, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	/* 'g' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x7c, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x04, 0x04, 0x78,
	/* 'h' */	6, 11, 0, 9, 7,
			0x80, 0x80, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x00, 0x00,
	/* 'i' */	2, 11, 0, 9, 4,
			0x40, 0x00, 0xc0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00,
	/* 'j' */	3, 11, 0, 9, 4,
			0x20, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xc0,
	/* 'k' */	7, 11, 0, 9, 8,
			0x40, 0x40, 0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00, 0x00,
	/* 'l' */	4, 11, 0, 9, 5,
			0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x30, 0x00, 0x00,
	/* 'm' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0xfc, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00, 0x00,
	/* 'n' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x00, 0x00,
	/* 'o' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x78, 0x84, 0x84, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00,
	/* 'p' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0xf8, 0x84, 0x84, 0x84, 0x84, 0x84, 0xf8, 0x80, 0x80,
	/* 'q' */       6, 11, 0, 9, 7,
			0x00, 0x00, 0x7c, 0x84, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x04, 0x04,
	/* 'r' */       5, 11, 0, 9, 6,
			0x00, 0x00, 0xb8, 0xc0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 's' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x7c, 0x80, 0x80, 0x7c, 0x02, 0x02, 0x7c, 0x00, 0x00,
	/* 't' */       6, 11, 0, 9, 7,
			0x00, 0x20, 0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1c, 0x00, 0x00,
	/* 'u' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x00, 0x00,
	/* 'v' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x84, 0x84, 0x84, 0x48, 0x48, 0x48, 0x30, 0x00, 0x00,
	/* 'w' */	7, 11, 0, 9, 8,
			0x00, 0x00, 0x82, 0x82, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00, 0x00,
	/* 'x' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x84, 0x84, 0x48, 0x30, 0x48, 0x84, 0x84, 0x00, 0x00,
	/* 'y' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0x84, 0x84, 0x84, 0x84, 0x84, 0x7c, 0x04, 0x04, 0x78,
	/* 'z' */	6, 11, 0, 9, 7,
			0x00, 0x00, 0xfc, 0x08, 0x10, 0x20, 0x40, 0x80, 0xfc, 0x00, 0x00,
	/* '{' */	5, 11, 0, 9, 6,
			0x18, 0x20, 0x20, 0x20, 0xc0, 0x20, 0x20, 0x20, 0x18, 0x00, 0x00,
	/* '|' */	1, 11, 0, 9, 2,
			0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* '}' */	5, 11, 0, 9, 6,
			0xc0, 0x20, 0x20, 0x20, 0x18, 0x20, 0x20, 0x20, 0xc0, 0x00, 0x00,
	/* '~' */	6, 11, 0, 9, 7,
			0x64, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static struct gp_font_face font_gfxprim = {
	.family_name = "Gfxprim",
	.style_name = "Proportional",
	.charset = GP_CHARSET_7BIT,
	.ascend  = 9,
	.descend = 2,
	.max_glyph_width = 9,
	.max_glyph_advance = 9,
	.glyph_bitmap_format = GP_FONT_BITMAP_1BPP,
	.glyphs = default_proportional_glyphs,
	.glyph_offsets = {16},
};

const gp_font_face *gp_font_gfxprim = &font_gfxprim;
