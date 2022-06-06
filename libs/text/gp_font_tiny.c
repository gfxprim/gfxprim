// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <text/gp_font.h>

static int8_t tiny_glyphs[] = {
	/* ' ' */ 	4, 5, 0, 5, 4,
	         	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '!' */ 	1, 5, 1, 5, 3,
	         	0x80, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00,
	/* '"' */	4, 5, 0, 5, 6,
	         	0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '#' */	5, 5, 0, 5, 6,
	        	0x50, 0xf8, 0x50, 0xf8, 0x50, 0x00, 0x00,
	/* '$' */ 	3, 5, 0, 5, 4,
	         	0x40, 0xe0, 0x40, 0xe0, 0x40, 0x00, 0x00,
	/* '%' */  	5, 5, 0, 5, 6,
	         	0xd0, 0x90, 0x20, 0x48, 0x58, 0x00, 0x00,
	/* '&' */  	4, 5, 0, 5, 5,
	         	0x60, 0x90, 0x40, 0xa0, 0xd0, 0x00, 0x00,
	/* ''' */  	1, 5, 0, 5, 2,
	         	0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '(' */  	2, 5, 0, 5, 3,
	         	0x40, 0x80, 0x80, 0x80, 0x40, 0x00, 0x00,
	/* ')' */  	2, 5, 0, 5, 3,
	         	0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00,
	/* '*' */  	3, 5, 0, 5, 4,
	         	0x00, 0xa0, 0x40, 0xa0, 0x00, 0x00, 0x00,
	/* '+' */  	3, 5, 0, 5, 4,
	         	0x00, 0x40, 0xe0, 0x40, 0x00, 0x00, 0x00,
	/* ',' */  	1, 6, 0, 5, 2,
	         	0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00,
	/* '-' */  	3, 5, 0, 5, 4,
	         	0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
	/* '.' */  	1, 5, 0, 5, 2,
	         	0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
	/* '/' */  	3, 5, 0, 5, 4,
	         	0x20, 0x20, 0x40, 0x80, 0x80, 0x00, 0x00,
	/* '0' */  	4, 5, 0, 5, 5,
	         	0x60, 0xb0, 0x90, 0xd0, 0x60, 0x00, 0x00,
	/* '1' */  	3, 5, 0, 5, 4,
	         	0xc0, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00,
	/* '2' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x10, 0x60, 0x80, 0xf0, 0x00, 0x00,
	/* '3' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x10, 0x60, 0x10, 0xe0, 0x00, 0x00,
	/* '4' */  	4, 5, 0, 5, 5,
	         	0x20, 0x60, 0xa0, 0xf0, 0x20, 0x00, 0x00,
	/* '5' */  	4, 5, 0, 5, 5,
	         	0xf0, 0x80, 0xe0, 0x10, 0xe0, 0x00, 0x00,
	/* '6' */  	4, 5, 0, 5, 5,
	         	0x60, 0x80, 0xe0, 0x90, 0x60, 0x00, 0x00,
	/* '7' */ 	4, 5, 0, 5, 5,
	         	0xf0, 0x10, 0x20, 0x40, 0x40, 0x00, 0x00,
	/* '8' */  	4, 5, 0, 5, 5,
	         	0x60, 0x90, 0x60, 0x90, 0x60, 0x00, 0x00,
	/* '9' */  	4, 5, 0, 5, 5,
	                0x60, 0x90, 0x70, 0x10, 0x60, 0x00, 0x00,
	/* ':' */  	1, 5, 0, 5, 2,
	         	0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00,
	/* ';' */  	1, 5, 0, 5, 2,
	         	0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00,
	/* '<' */  	3, 5, 0, 5, 4,
	         	0x20, 0x40, 0x80, 0x40, 0x20, 0x00, 0x00,
	/* '=' */  	3, 5, 0, 5, 4,
	         	0x00, 0xe0, 0x00, 0xe0, 0x00, 0x00, 0x00,
	/* '>' */  	3, 5, 0, 5, 4,
	         	0x80, 0x40, 0x20, 0x40, 0x80, 0x00, 0x00,
	/* '?' */  	3, 5, 0, 5, 4,
	         	0xc0, 0x20, 0x40, 0x00, 0x40, 0x00, 0x00,
	/* '@' */  	5, 5, 0, 5, 6,
	         	0x70, 0x88, 0xb0, 0x80, 0x70, 0x00, 0x00,
	/* 'A' */  	4, 5, 0, 5, 5,
	         	0x60, 0x90, 0xf0, 0x90, 0x90, 0x00, 0x00,
	/* 'B' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x90, 0xe0, 0x90, 0xe0, 0x00, 0x00,
	/* 'C' */  	4, 5, 0, 5, 5,
	         	0x70, 0x80, 0x80, 0x80, 0x70, 0x00, 0x00,
	/* 'D' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x90, 0x90, 0x90, 0xe0, 0x00, 0x00,
	/* 'E' */  	4, 5, 0, 5, 5,
	         	0xf0, 0x80, 0xe0, 0x80, 0xf0, 0x00, 0x00,
	/* 'F' */  	4, 5, 0, 5, 5,
	         	0xf0, 0x80, 0xe0, 0x80, 0x80, 0x00, 0x00,
	/* 'G' */  	5, 5, 0, 5, 6,
	         	0x70, 0x80, 0x98, 0x88, 0x70, 0x00, 0x00,
	/* 'H' */  	4, 5, 0, 5, 5,
	         	0x90, 0x90, 0xf0, 0x90, 0x90, 0x00, 0x00,
	/* 'I' */  	3, 5, 0, 5, 4,
	         	0xe0, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00,
	/* 'J' */  	4, 5, 0, 5, 5,
	         	0xf0, 0x10, 0x10, 0x90, 0x60, 0x00, 0x00,
	/* 'K' */  	4, 5, 0, 5, 5,
	         	0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x00, 0x00,
	/* 'L' */  	4, 5, 0, 5, 5,
	         	0x80, 0x80, 0x80, 0x80, 0xf0, 0x00, 0x00,
	/* 'M' */  	5, 5, 0, 5, 6,
	         	0x88, 0xd8, 0xa8, 0x88, 0x88, 0x00, 0x00,
	/* 'N' */  	4, 5, 0, 5, 5,
	         	0x90, 0xd0, 0xb0, 0x90, 0x90, 0x00, 0x00,
	/* 'O' */  	4, 5, 0, 5, 5,
	         	0x60, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'P' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x90, 0xe0, 0x80, 0x80, 0x00, 0x00,
	/* 'Q' */  	4, 5, 0, 5, 5,
	         	0x60, 0x90, 0x90, 0xa0, 0x50, 0x00, 0x00,
	/* 'R' */  	4, 5, 0, 5, 5,
	         	0xe0, 0x90, 0xe0, 0x90, 0x90, 0x00, 0x00,
	/* 'S' */  	4, 5, 0, 5, 5,
	         	0x70, 0x80, 0x60, 0x10, 0xe0, 0x00, 0x00,
	/* 'T' */  	5, 5, 0, 5, 6,
	         	0xf8, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	/* 'U' */  	4, 5, 0, 5, 5,
	         	0x90, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'V' */  	5, 5, 0, 5, 6,
	         	0x88, 0x88, 0x50, 0x50, 0x20, 0x00, 0x00,
	/* 'W' */  	5, 5, 0, 5, 6,
	         	0x88, 0x88, 0x88, 0xa8, 0x50, 0x00, 0x00,
	/* 'X' */  	4, 5, 0, 5, 5,
	         	0x90, 0x90, 0x60, 0x90, 0x90, 0x00, 0x00,
	/* 'Y' */  	4, 5, 0, 5, 5,
	         	0x90, 0x90, 0x70, 0x10, 0x60, 0x00, 0x00,
	/* 'Z' */  	4, 5, 0, 5, 5,
	         	0xf0, 0x10, 0x60, 0x80, 0xf0, 0x00, 0x00,
	/* '[' */  	2, 5, 0, 5, 3,
	         	0xc0, 0x80, 0x80, 0x80, 0xc0, 0x00, 0x00,
	/* '\' */  	3, 5, 0, 5, 4,
	         	0x80, 0x80, 0x40, 0x20, 0x20, 0x00, 0x00,
	/* ']' */  	2, 5, 0, 5, 3,
	         	0xc0, 0x40, 0x40, 0x40, 0xc0, 0x00, 0x00,
	/* '^' */  	3, 5, 0, 5, 4,
	         	0x40, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '_' */  	4, 5, 0, 5, 5,
	         	0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
	/* '`' */  	2, 5, 0, 5, 3,
	         	0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 'a' */  	4, 5, 0, 5, 5,
	         	0x00, 0x60, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'b' */  	4, 5, 0, 5, 5,
	         	0x80, 0xe0, 0x90, 0x90, 0xe0, 0x00, 0x00,
	/* 'c' */  	4, 5, 0, 5, 5,
	         	0x00, 0x70, 0x80, 0x80, 0x70, 0x00, 0x00,
	/* 'd' */  	4, 5, 0, 5, 5,
	         	0x10, 0x70, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'e' */  	4, 5, 0, 5, 5,
	         	0x00, 0x60, 0xf0, 0x80, 0x70, 0x00, 0x00,
	/* 'f' */  	4, 7, 0, 5, 5,
	         	0x00, 0x70, 0x80, 0xc0, 0x80, 0x80, 0x80,
	/* 'g' */  	4, 7, 0, 5, 5,
	         	0x00, 0x60, 0x90, 0x90, 0x70, 0x10, 0x60,
	/* 'h' */  	4, 5, 0, 5, 5,
	                0x80, 0xe0, 0x90, 0x90, 0x90, 0x00, 0x00,
	/* 'i' */  	3, 5, 0, 5, 4,
	         	0x40, 0x00, 0x40, 0x40, 0xe0, 0x00, 0x00,
	/* 'j' */  	4, 5, 0, 5, 5,
	         	0x00, 0x10, 0x10, 0x90, 0x60, 0x00, 0x00,
	/* 'k' */  	4, 5, 0, 5, 5,
	         	0x80, 0xb0, 0xc0, 0xa0, 0x90, 0x00, 0x00,
	/* 'l' */  	2, 5, 0, 5, 3,
	         	0x80, 0x80, 0x80, 0x80, 0x40, 0x00, 0x00,
	/* 'm' */  	5, 5, 0, 5, 6,
	         	0x00, 0xd0, 0xa8, 0xa8, 0xa8, 0x00, 0x00,
	/* 'n' */  	4, 5, 0, 5, 5,
	                0x00, 0xe0, 0x90, 0x90, 0x90, 0x00, 0x00,
	/* 'o' */  	4, 5, 0, 5, 5,
	         	0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'p' */  	4, 7, 0, 5, 5,
	         	0x00, 0xe0, 0x90, 0x90, 0xe0, 0x80, 0x80,
	/* 'q' */       4, 7, 0, 5, 5,
	         	0x00, 0x70, 0x90, 0x90, 0x70, 0x10, 0x10,
	/* 'r' */       4, 5, 0, 5, 5,
	         	0x00, 0x70, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 's' */	4, 5, 0, 5, 5,
	         	0x00, 0x70, 0xc0, 0x30, 0xe0, 0x00, 0x00,
	/* 't' */       4, 5, 0, 5, 5,
	         	0x40, 0xf0, 0x40, 0x40, 0x30, 0x00, 0x00,
	/* 'u' */	4, 5, 0, 5, 5,
	         	0x00, 0x90, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'v' */	4, 5, 0, 5, 5,
	         	0x00, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'w' */	5, 5, 0, 5, 6,
	         	0x00, 0x88, 0xa8, 0xa8, 0x50, 0x00, 0x00,
	/* 'x' */	4, 5, 0, 5, 5,
	         	0x00, 0x90, 0x60, 0x60, 0x90, 0x00, 0x00,
	/* 'y' */	4, 7, 0, 5, 5,
	         	0x00, 0x90, 0x90, 0x90, 0x70, 0x10, 0x60,
	/* 'z' */	4, 5, 0, 5, 5,
	         	0x00, 0xf0, 0x20, 0x40, 0xf0, 0x00, 0x00,
	/* '{' */	3, 5, 0, 5, 4,
	         	0x60, 0x80, 0x40, 0x80, 0x60, 0x00, 0x00,
	/* '|' */	1, 5, 0, 5, 2,
	         	0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* '}' */	3, 5, 0, 5, 4,
	         	0xc0, 0x20, 0x40, 0x20, 0xc0, 0x00, 0x00,
	/* '~' */	4, 5, 0, 5, 5,
	         	0x00, 0x50, 0xa0, 0x00, 0x00, 0x00, 0x00,
};

static struct gp_font_face tiny = {
	.family_name = "Tiny",
	.style = GP_FONT_REGULAR,
	.ascend  = 5,
	.descend = 3,
	.max_glyph_width = 5,
	.max_glyph_advance = 6,
	.glyph_bitmap_format = GP_FONT_BITMAP_1BPP,
	.glyphs = {
		{
			.min_glyph = 0x20,
			.max_glyph = 0x7f,
			.glyphs = tiny_glyphs,
			.offset = 12,
		}
	}
};

static int8_t tiny_mono_glyphs[] = {
	/* ' ' */	5, 5, 0, 5, 6,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '!' */	5, 5, 0, 5, 6,
			0x20, 0x20, 0x20, 0x00, 0x20, 0x00, 0x00,
	/* '"' */	5, 5, 0, 5, 6,
			0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '#' */	5, 5, 0, 5, 6,
			0x50, 0xf8, 0x50, 0xf8, 0x50, 0x00, 0x00,
	/* '$' */	5, 5, 0, 5, 6,
			0x20, 0x70, 0x20, 0x70, 0x20, 0x00, 0x00,
	/* '%' */	5, 5, 0, 5, 6,
			0xd0, 0x90, 0x20, 0x48, 0x58, 0x00, 0x00,
	/* '&' */	5, 5, 0, 5, 6,
			0x60, 0x90, 0x40, 0xa0, 0xd0, 0x00, 0x00,
	/* ''' */	5, 5, 0, 5, 6,
			0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '(' */	5, 5, 0, 5, 6,
			0x20, 0x40, 0x40, 0x40, 0x20, 0x00, 0x00,
	/* ')' */	5, 5, 0, 5, 6,
			0x20, 0x10, 0x10, 0x10, 0x20, 0x00, 0x00,
	/* '*' */	5, 5, 0, 5, 6,
			0x00, 0x50, 0x20, 0x50, 0x00, 0x00, 0x00,
	/* '+' */	5, 5, 0, 5, 6,
			0x00, 0x20, 0x70, 0x20, 0x00, 0x00, 0x00,
	/* ',' */	5, 6, 0, 5, 6,
			0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00,
	/* '-' */	5, 5, 0, 5, 6,
			0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
	/* '.' */	5, 5, 0, 5, 6,
			0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,
	/* '/' */	5, 5, 0, 5, 6,
			0x10, 0x10, 0x20, 0x40, 0x40, 0x00, 0x00,
	/* '0' */	5, 5, 0, 5, 6,
			0x60, 0xb0, 0x90, 0xd0, 0x60, 0x00, 0x00,
	/* '1' */	5, 5, 0, 5, 6,
			0x60, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
	/* '2' */	5, 5, 0, 5, 6,
			0xe0, 0x10, 0x60, 0x80, 0xf0, 0x00, 0x00,
	/* '3' */	5, 5, 0, 5, 6,
			0xe0, 0x10, 0x60, 0x10, 0xe0, 0x00, 0x00,
	/* '4' */	5, 5, 0, 5, 6,
			0x20, 0x60, 0xa0, 0xf0, 0x20, 0x00, 0x00,
	/* '5' */	5, 5, 0, 5, 6,
			0xf0, 0x80, 0xe0, 0x10, 0xe0, 0x00, 0x00,
	/* '6' */	5, 5, 0, 5, 6,
			0x60, 0x80, 0xe0, 0x90, 0x60, 0x00, 0x00,
	/* '7' */	5, 5, 0, 5, 6,
			0xf0, 0x10, 0x20, 0x40, 0x40, 0x00, 0x00,
	/* '8' */	5, 5, 0, 5, 6,
			0x60, 0x90, 0x60, 0x90, 0x60, 0x00, 0x00,
	/* '9' */	5, 5, 0, 5, 6,
	                0x60, 0x90, 0x70, 0x10, 0x60, 0x00, 0x00,
	/* ':' */	5, 5, 0, 5, 6,
			0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00,
	/* ';' */	5, 5, 0, 5, 6,
			0x00, 0x20, 0x00, 0x20, 0x20, 0x00, 0x00,
	/* '<' */	5, 5, 0, 5, 6,
			0x10, 0x20, 0x40, 0x20, 0x10, 0x00, 0x00,
	/* '=' */	5, 5, 0, 5, 6,
			0x00, 0x70, 0x00, 0x70, 0x00, 0x00, 0x00,
	/* '>' */	5, 5, 0, 5, 6,
			0x40, 0x20, 0x10, 0x20, 0x40, 0x00, 0x00,
	/* '?' */	5, 5, 0, 5, 6,
			0x60, 0x10, 0x20, 0x00, 0x20, 0x00, 0x00,
	/* '@' */	5, 5, 0, 5, 6,
			0x70, 0x88, 0xb0, 0x80, 0x70, 0x00, 0x00,
	/* 'A' */	5, 5, 0, 5, 6,
			0x60, 0x90, 0xf0, 0x90, 0x90, 0x00, 0x00,
	/* 'B' */	5, 5, 0, 5, 6,
			0xe0, 0x90, 0xe0, 0x90, 0xe0, 0x00, 0x00,
	/* 'C' */	5, 5, 0, 5, 6,
			0x70, 0x80, 0x80, 0x80, 0x70, 0x00, 0x00,
	/* 'D' */	5, 5, 0, 5, 6,
			0xe0, 0x90, 0x90, 0x90, 0xe0, 0x00, 0x00,
	/* 'E' */	5, 5, 0, 5, 6,
			0xf0, 0x80, 0xe0, 0x80, 0xf0, 0x00, 0x00,
	/* 'F' */	5, 5, 0, 5, 6,
			0xf0, 0x80, 0xe0, 0x80, 0x80, 0x00, 0x00,
	/* 'G' */	5, 5, 0, 5, 6,
			0x70, 0x80, 0x98, 0x88, 0x70, 0x00, 0x00,
	/* 'H' */	5, 5, 0, 5, 6,
			0x90, 0x90, 0xf0, 0x90, 0x90, 0x00, 0x00,
	/* 'I' */	5, 5, 0, 5, 6,
			0x70, 0x20, 0x20, 0x20, 0x70, 0x00, 0x00,
	/* 'J' */	5, 5, 0, 5, 6,
			0xf0, 0x10, 0x10, 0x90, 0x60, 0x00, 0x00,
	/* 'K' */	5, 5, 0, 5, 6,
			0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x00, 0x00,
	/* 'L' */	5, 5, 0, 5, 6,
			0x80, 0x80, 0x80, 0x80, 0xf0, 0x00, 0x00,
	/* 'M' */	5, 5, 0, 5, 6,
			0x88, 0xd8, 0xa8, 0x88, 0x88, 0x00, 0x00,
	/* 'N' */	5, 5, 0, 5, 6,
			0x90, 0xd0, 0xb0, 0x90, 0x90, 0x00, 0x00,
	/* 'O' */	5, 5, 0, 5, 6,
			0x60, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'P' */	5, 5, 0, 5, 6,
			0xe0, 0x90, 0xe0, 0x80, 0x80, 0x00, 0x00,
	/* 'Q' */	5, 5, 0, 5, 6,
			0x60, 0x90, 0x90, 0xa0, 0x50, 0x00, 0x00,
	/* 'R' */	5, 5, 0, 5, 6,
			0xe0, 0x90, 0xe0, 0x90, 0x90, 0x00, 0x00,
	/* 'S' */	5, 5, 0, 5, 6,
			0x70, 0x80, 0x60, 0x10, 0xe0, 0x00, 0x00,
	/* 'T' */	5, 5, 0, 5, 6,
			0xf8, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	/* 'U' */	5, 5, 0, 5, 6,
			0x90, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'V' */	5, 5, 0, 5, 6,
			0x88, 0x88, 0x50, 0x50, 0x20, 0x00, 0x00,
	/* 'W' */	5, 5, 0, 5, 6,
			0x88, 0x88, 0x88, 0xa8, 0x50, 0x00, 0x00,
	/* 'X' */	5, 5, 0, 5, 6,
			0x90, 0x90, 0x60, 0x90, 0x90, 0x00, 0x00,
	/* 'Y' */	5, 5, 0, 5, 6,
			0x90, 0x90, 0x70, 0x10, 0x60, 0x00, 0x00,
	/* 'Z' */	5, 5, 0, 5, 6,
			0xf0, 0x10, 0x60, 0x80, 0xf0, 0x00, 0x00,
	/* '[' */	5, 5, 0, 5, 6,
			0x60, 0x40, 0x40, 0x40, 0x60, 0x00, 0x00,
	/* '\' */	5, 5, 0, 5, 6,
			0x40, 0x40, 0x20, 0x10, 0x10, 0x00, 0x00,
	/* ']' */	5, 5, 0, 5, 6,
			0x30, 0x10, 0x10, 0x10, 0x30, 0x00, 0x00,
	/* '^' */	5, 5, 0, 5, 6,
			0x20, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* '_' */	5, 5, 0, 5, 6,
			0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
	/* '`' */	5, 5, 0, 5, 6,
			0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 'a' */	5, 5, 0, 5, 6,
			0x00, 0x60, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'b' */	5, 5, 0, 5, 6,
			0x80, 0xe0, 0x90, 0x90, 0xe0, 0x00, 0x00,
	/* 'c' */	5, 5, 0, 5, 6,
			0x00, 0x70, 0x80, 0x80, 0x70, 0x00, 0x00,
	/* 'd' */	5, 5, 0, 5, 6,
			0x10, 0x70, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'e' */	5, 5, 0, 5, 6,
			0x00, 0x60, 0xf0, 0x80, 0x70, 0x00, 0x00,
	/* 'f' */	5, 7, 0, 5, 6,
			0x00, 0x70, 0x80, 0xc0, 0x80, 0x80, 0x80,
	/* 'g' */	5, 7, 0, 5, 6,
			0x00, 0x60, 0x90, 0x90, 0x70, 0x10, 0x60,
	/* 'h' */	5, 5, 0, 5, 6,
	                0x80, 0xe0, 0x90, 0x90, 0x90, 0x00, 0x00,
	/* 'i' */	5, 5, 0, 5, 6,
			0x20, 0x00, 0x20, 0x20, 0x70, 0x00, 0x00,
	/* 'j' */	5, 5, 0, 5, 6,
			0x00, 0x10, 0x10, 0x90, 0x60, 0x00, 0x00,
	/* 'k' */	5, 5, 0, 5, 6,
			0x80, 0xb0, 0xc0, 0xa0, 0x90, 0x00, 0x00,
	/* 'l' */	5, 5, 0, 5, 6,
			0x40, 0x40, 0x40, 0x40, 0x30, 0x00, 0x00,
	/* 'm' */	5, 5, 0, 5, 6,
			0x00, 0xd0, 0xa8, 0xa8, 0xa8, 0x00, 0x00,
	/* 'n' */	5, 5, 0, 5, 6,
	                0x00, 0xe0, 0x90, 0x90, 0x90, 0x00, 0x00,
	/* 'o' */	5, 5, 0, 5, 6,
			0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'p' */	5, 7, 0, 5, 6,
			0x00, 0xe0, 0x90, 0x90, 0xe0, 0x80, 0x80,
	/* 'q' */       5, 7, 0, 5, 6,
			0x00, 0x70, 0x90, 0x90, 0x70, 0x10, 0x10,
	/* 'r' */       5, 5, 0, 5, 6,
			0x00, 0x70, 0x80, 0x80, 0x80, 0x00, 0x00,
	/* 's' */	5, 5, 0, 5, 6,
			0x00, 0x70, 0xc0, 0x30, 0xe0, 0x00, 0x00,
	/* 't' */       5, 5, 0, 5, 6,
			0x40, 0xf0, 0x40, 0x40, 0x30, 0x00, 0x00,
	/* 'u' */	5, 5, 0, 5, 6,
			0x00, 0x90, 0x90, 0x90, 0x70, 0x00, 0x00,
	/* 'v' */	5, 5, 0, 5, 6,
			0x00, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00,
	/* 'w' */	5, 5, 0, 5, 6,
			0x00, 0x88, 0xa8, 0xa8, 0x50, 0x00, 0x00,
	/* 'x' */	5, 5, 0, 5, 6,
			0x00, 0x90, 0x60, 0x60, 0x90, 0x00, 0x00,
	/* 'y' */	5, 7, 0, 5, 6,
			0x00, 0x90, 0x90, 0x90, 0x70, 0x10, 0x60,
	/* 'z' */	5, 5, 0, 5, 6,
			0x00, 0xf0, 0x20, 0x40, 0xf0, 0x00, 0x00,
	/* '{' */	5, 5, 0, 5, 6,
			0x30, 0x40, 0x20, 0x40, 0x30, 0x00, 0x00,
	/* '|' */	5, 5, 0, 5, 6,
			0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
	/* '}' */	5, 5, 0, 5, 6,
			0x60, 0x10, 0x20, 0x10, 0x60, 0x00, 0x00,
	/* '~' */	5, 5, 0, 5, 6,
			0x00, 0x50, 0xa0, 0x00, 0x00, 0x00, 0x00,
};

static struct gp_font_face tiny_mono = {
	.family_name = "Tiny",
	.style = GP_FONT_MONO,
	.ascend  = 5,
	.descend = 3,
	.max_glyph_width = 5,
	.max_glyph_advance = 6,
	.glyph_bitmap_format = GP_FONT_BITMAP_1BPP,
	.glyphs = {
		{
			.min_glyph = 0x20,
			.max_glyph = 0x7f,
			.glyphs = tiny_mono_glyphs,
			.offset = 12,
		}
	}
};

const gp_font_family __attribute__((visibility ("hidden"))) font_family_tiny = {
	.family_name = "tiny",
	.fonts = {
		&tiny,
		&tiny_mono,
		NULL
	}
};
