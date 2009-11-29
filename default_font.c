/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GP.h"

static uint8_t GP_default_font_data[] = {

	/* ' ' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* '!' */
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0, 0, 0x08,

	/* '"' */
	0b00100100,
	0b00100100,
	0b01001000,
	0, 0, 0, 0, 0, 0, 0,

	/* '#' */
	0b00100100,
	0b00100100,
	0b11111111,
	0b00100100,
	0b00100100,
	0b00100100,
	0b00100100,
	0b11111111,
	0b00100100,
	0b00100100,

	/* '$' */
	0b00001000,
	0b00111110,
	0b01001001,
	0b01001000,
	0b00111000,
	0b00001110,
	0b00001001,
	0b01001001,
	0b00111110,
	0b00001000,

	/* '%' */
	0b01100000,
	0b10010001,
	0b10010010,
	0b01100100,
	0b00001000,
	0b00010000,
        0b00100110,
        0b01001001,
        0b10001001,
        0b00000110,

	/* '&' */
	0b00011000,
	0b00100100,
	0b00100100,
	0b00100100,
	0b00011000,
	0b00101011,
	0b01000100,
	0b01000100,
	0b01000100,
	0b00111011,

	/* '`' */ 0x10, 0x08, 0x04, 0, 0, 0, 0, 0, 0, 0,
	/* '(' */ 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x08,
	/* ')' */ 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10,

	/* '*' */
	0b00001000,
	0b01101011,
	0b00011100,
	0b01101011,
	0b00001000,
	0, 0, 0, 0, 0,

	/* '+' */
	0, 0, 0,
	0b00001000,
	0b00001000,
	0b01111111,
	0b00001000,
	0b00001000,
	0, 0,

	/* ',' */ 0, 0, 0, 0, 0, 0, 0, 0x08, 0x08, 0x10,
	/* '-' */ 0, 0, 0, 0, 0, 0x7f, 0, 0, 0, 0,
	/* '.' */ 0, 0, 0, 0, 0, 0, 0, 0x18, 0x18, 0,

	/* '/' */
	0,
	0b00000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b10000000,
	0,

	/* '0' */
	0,
	0b00111110,
	0b01000011,
	0b01000101,
	0b01001001,
	0b01001001,
	0b01010001,
	0b01100001,
	0b00111110,
	0,

	/* '1' */
	0,
	0b00001000,
	0b00111000,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,
	0b01111111,
	0,

	/* '2' */
	0,
	0b00111110,
	0b01000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01111111,
	0,

	/* '3' */
	0,
	0b00111110,
	0b01000001,
	0b00000001,
	0b00001110,
	0b00000001,
	0b00000001,
	0b01000001,
	0b00111110,
	0,

	/* '4' */
	0,
	0b00001100,
	0b00010100,
	0b00100100,
	0b01000100,
	0b10000100,
	0b11111111,
	0b00000100,
	0b00000100,
	0,

	/* '5' */
	0,
	0b01111111,
	0b01000000,
	0b01000000,
	0b01111110,
	0b00000001,
	0b00000001,
	0b01000001,
	0b00111110,
	0,

	/* '6' */
	0,
	0b00111111,
	0b01000000,
	0b01000000,
	0b01111110,
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,
	0,

	/* '7' */
	0,
	0b01111111,
	0b01000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,
	0,

	/* '8' */
	0,
	0b00111110,
	0b01000001,
	0b01000001,
	0b00111110,
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,
	0,

	/* '9' */
	0,
	0b00111110,
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111111,
	0b00000001,
	0b01000001,
	0b00111110,
	0,

	/* ':' */  0, 0, 0, 0x18, 0x18, 0, 0, 0x18, 0x18, 0,
	/* ';' */  0, 0, 0, 0x18, 0x18, 0, 0, 0x18, 0x18, 0x30,

	/* '<' */
	0, 0,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b00100000,
	0b00010000,
	0b00001000,
	0,

	/* '=' */ 0, 0, 0, 0x7f, 0, 0, 0x7f, 0, 0, 0,

	/* '>' */
	0, 0,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00010000,
	0,

	/* '?' */
	0b00111110,
	0b01000001,
	0b00000001,
	0b00000010,
	0b00000100,
	0b00001000,
	0b00001000,
	0b00000000,
	0b00000000,
	0b00001000,
	0,
};

GP_Font GP_default_font = {
	.charset = GP_CHARSET_7BIT,
	.data = GP_default_font_data,
	.char_width = 8,
	.hspace = 1,
	.height = 10,
	.bytes_per_line = 1,
};

