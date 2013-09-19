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

#include "GP_Font.h"

static int8_t c64_glyphs[] = {
	/* ' ' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '!' */      8, 7, 0, 7, 8,
	               0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x30,
	/* '"' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x00,
	/* '#' */      8, 7, -1, 7, 8,
	               0x66, 0x66, 0xff, 0x66, 0xff, 0x66, 0x66,
	/* '$' */      8, 7, 0, 7, 8,
	               0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30,
	/* '%' */      8, 7, 0, 7, 8,
	               0xc4, 0xcc, 0x18, 0x30, 0x60, 0xcc, 0x8c,
	/* '&' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0x78, 0x70, 0xce, 0xcc, 0x7e,
	/* ''' */      8, 7, 0, 7, 8,
	               0x0c, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00,
	/* '(' */      8, 7, 0, 7, 8,
	               0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18,
	/* ')' */      8, 7, 0, 7, 8,
	               0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60,
	/* '*' */      8, 7, -1, 7, 8,
	               0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00,
	/* '+' */      8, 7, 0, 7, 8,
	               0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00,
	/* ',' */      8, 7, 0, 6, 8,
	               0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60,
	/* '-' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00,
	/* '.' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30,
	/* '/' */      8, 7, 0, 7, 8,
	               0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0,
	/* '0' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xdc, 0xec, 0xcc, 0xcc, 0x78,
	/* '1' */      8, 7, 0, 7, 8,
	               0x30, 0x30, 0x70, 0x30, 0x30, 0x30, 0xfc,
	/* '2' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0x0c, 0x18, 0x60, 0xc0, 0xfc,
	/* '3' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78,
	/* '4' */      8, 7, 0, 7, 8,
	               0x0c, 0x1c, 0x3c, 0xcc, 0xfe, 0x0c, 0x0c,
	/* '5' */      8, 7, 0, 7, 8,
	               0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78,
	/* '6' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xc0, 0xf8, 0xcc, 0xcc, 0x78,
	/* '7' */      8, 7, 0, 7, 8,
	               0xfc, 0xcc, 0x18, 0x30, 0x30, 0x30, 0x30,
	/* '8' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78,
	/* '9' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0xcc, 0x78,
	/* ':' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00,
	/* ';' */      8, 7, 0, 6, 8,
	               0x00, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60,
	/* '<' */      8, 7, 0, 7, 8,
	               0x1c, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x1c,
	/* '=' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0x00,
	/* '>' */      8, 7, 0, 7, 8,
	               0xe0, 0x30, 0x18, 0x0c, 0x18, 0x30, 0xe0,
	/* '?' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30,
	/* '@' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xdc, 0xdc, 0xc0, 0xc4, 0x78,
	/* 'A' */      8, 7, 0, 7, 8,
	               0x30, 0x78, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc,
	/* 'B' */      8, 7, 0, 7, 8,
	               0xf8, 0xcc, 0xcc, 0xf8, 0xcc, 0xcc, 0xf8,
	/* 'C' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xc0, 0xc0, 0xc0, 0xcc, 0x78,
	/* 'D' */      8, 7, 0, 7, 8,
	               0xf0, 0xd8, 0xcc, 0xcc, 0xcc, 0xd8, 0xf0,
	/* 'E' */      8, 7, 0, 7, 8,
	               0xfc, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xfc,
	/* 'F' */      8, 7, 0, 7, 8,
	               0xfc, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xc0,
	/* 'G' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xc0, 0xdc, 0xcc, 0xcc, 0x78,
	/* 'H' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc,
	/* 'I' */      8, 7, 0, 7, 8,
	               0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78,
	/* 'J' */      8, 7, 0, 7, 8,
	               0x3c, 0x18, 0x18, 0x18, 0x18, 0xd8, 0x70,
	/* 'K' */      8, 7, 0, 7, 8,
	               0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc,
	/* 'L' */      8, 7, 0, 7, 8,
	               0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc,
	/* 'M' */      8, 7, 0, 7, 8,
	               0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6,
	/* 'N' */      8, 7, 0, 7, 8,
	               0xcc, 0xec, 0xfc, 0xfc, 0xdc, 0xcc, 0xcc,
	/* 'O' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78,
	/* 'P' */      8, 7, 0, 7, 8,
	               0xf8, 0xcc, 0xcc, 0xf8, 0xc0, 0xc0, 0xc0,
	/* 'Q' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x1c,
	/* 'R' */      8, 7, 0, 7, 8,
	               0xf8, 0xcc, 0xcc, 0xf8, 0xf0, 0xd8, 0xcc,
	/* 'S' */      8, 7, 0, 7, 8,
	               0x78, 0xcc, 0xc0, 0x78, 0x0c, 0xcc, 0x78,
	/* 'T' */      8, 7, 0, 7, 8,
	               0xfc, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	/* 'U' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78,
	/* 'V' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30,
	/* 'W' */      8, 7, 0, 7, 8,
	               0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6,
	/* 'X' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0x78, 0x30, 0x78, 0xcc, 0xcc,
	/* 'Y' */      8, 7, 0, 7, 8,
	               0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x30,
	/* 'Z' */      8, 7, 0, 7, 8,
	               0xfc, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0xfc,
	/* '[' */      8, 7, 0, 7, 8,
	               0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78,
	/* '\' */      8, 7, 0, 7, 8,
	               0x00, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06,
	/* ']' */      8, 7, 0, 7, 8,
	               0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78,
	/* '^' */      8, 7, 0, 7, 8,
	               0x30, 0x78, 0xcc, 0x00, 0x00, 0x00, 0x00,
	/* '_' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
	/* '`' */      8, 7, 0, 7, 8,
	               0x30, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00,
	/* 'a' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7c,
	/* 'b' */      8, 7, 0, 7, 8,
	               0x00, 0xc0, 0xc0, 0xf8, 0xcc, 0xcc, 0xf8,
	/* 'c' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x78, 0xc0, 0xc0, 0xc0, 0x78,
	/* 'd' */      8, 7, 0, 7, 8,
	               0x00, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x7c,
	/* 'e' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78,
	/* 'f' */      8, 7, 0, 7, 8,
	               0x00, 0x38, 0x60, 0xf8, 0x60, 0x60, 0x60,
	/* 'g' */      8, 7, 0, 6, 8,
	               0x00, 0x7c, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
	/* 'h' */      8, 7, 0, 7, 8,
	               0x00, 0xc0, 0xc0, 0xf8, 0xcc, 0xcc, 0xcc,
	/* 'i' */      8, 7, 0, 7, 8,
	               0x00, 0x30, 0x00, 0x70, 0x30, 0x30, 0x78,
	/* 'j' */      8, 7, 0, 6, 8,
	               0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0xf0,
	/* 'k' */      8, 7, 0, 7, 8,
	               0x00, 0xc0, 0xc0, 0xd8, 0xf0, 0xd8, 0xcc,
	/* 'l' */      8, 7, 0, 7, 8,
	               0x00, 0x70, 0x30, 0x30, 0x30, 0x30, 0x78,
	/* 'm' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xc6,
	/* 'n' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc,
	/* 'o' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78,
	/* 'p' */      8, 7, 0, 6, 8,
	               0x00, 0xf8, 0xcc, 0xcc, 0xf8, 0xc0, 0xc0,
	/* 'q' */      8, 7, 0, 6, 8,
	               0x00, 0x7c, 0xcc, 0xcc, 0x7c, 0x0c, 0x0c,
	/* 'r' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xf8, 0xcc, 0xc0, 0xc0, 0xc0,
	/* 's' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8,
	/* 't' */      8, 7, 0, 7, 8,
	               0x00, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x1c,
	/* 'u' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x7c,
	/* 'v' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30,
	/* 'w' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xc6, 0xd6, 0xfe, 0x7c, 0x6c,
	/* 'x' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xcc, 0x78, 0x30, 0x78, 0xcc,
	/* 'y' */      8, 7, 0, 6, 8,
	               0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x18, 0xf0,
	/* 'z' */      8, 7, 0, 7, 8,
	               0x00, 0x00, 0xfc, 0x18, 0x30, 0x60, 0xfc,
	/* '{' */      8, 7, 0, 7, 8,
	               0x30, 0x60, 0x60, 0xc0, 0x60, 0x60, 0x30,
	/* '|' */      8, 7, 0, 7, 8,
	               0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	/* '}' */      8, 7, 0, 7, 8,
	               0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30,
	/* '~' */      8, 7, 0, 7, 8,
	               0x6c, 0xd8, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static struct GP_FontFace c64 = {
	.family_name = "C64",
	.style_name = "Mono",
	.charset = GP_CHARSET_7BIT,
	.ascend  = 7,
	.descend = 2,
	.max_glyph_width = 8,
	.max_glyph_advance = 9,
	.glyph_bitmap_format = GP_FONT_BITMAP_1BPP,
	.glyphs = c64_glyphs,
	.glyph_offsets = {12},
};

const struct GP_FontFace *GP_FontC64 = &c64;