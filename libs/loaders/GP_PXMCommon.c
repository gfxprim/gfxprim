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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdint.h>
#include <inttypes.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <GP_Debug.h>

#include "GP_PXMCommon.h"

static int read_comment(FILE *f)
{
	for (;;) {
		switch (fgetc(f)) {
		case '\n':
			return 0;
		case EOF:
			return 1;
		}
	}
}

int GP_PXMLoad1bpp(FILE *f, GP_Context *context)
{
	uint8_t *pixel = context->pixels;
	uint32_t x, y;

	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->w;) {
			int run = 1;
			
			while (run) {
				switch (fgetc(f)) {
				case EOF:
					return EIO;
				case '#':
					if (read_comment(f))
						return EIO;
					break;
				case '0':
					*pixel &= ~(0x80>>(x%8));
					run = 0;
					break;
				case '1':
					*pixel |= 0x80>>(x%8);
					run = 0;
					break;
				default:
					break;
				}
			}
			
			x++;
			
			if (x%8 == 0)
				pixel++;
		}

		if (context->w%8)
			pixel++;
	}

	return 0;
}

int GP_PXMLoad2bpp(FILE *f, GP_Context *context)
{
	uint8_t *pixel = context->pixels;
	uint32_t x, y;

	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->w;) {
			int run = 1;
			
			while (run) {
				switch (fgetc(f)) {
				case EOF:
					return EIO;
				case '#':
					if (read_comment(f))
						return EIO;
					break;
				case '0':
					*pixel &= ~(0xc0>>(2*(x%4)));
					run = 0;
					break;
				case '1':
					*pixel &= ~(0x80>>(2*(x%4)));
					*pixel |= 0x40>>(2*(x%4));
					run = 0;
					break;
				case '2':
					*pixel |= 0x80>>(2*(x%4));
					*pixel &= ~(0x40>>(2*(x%4)));
					run = 0;
					break;
				case '3':
					*pixel |= 0xc0>>(2*(x%4));
					run = 0;
					break;
				default:
					break;
				}
			}
			
			x++;
			
			if (x%4 == 0)
				pixel++;
		}

		if (context->w%4)
			pixel++;
	}

	return 0;
}

int GP_PXMLoad4bpp(FILE *f, GP_Context *context)
{
	uint8_t *pixel = context->pixels;
	uint32_t x, y;

	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->w;) {
			int run = 1;
			int val = 0;
			char ch;

			while (run) {
				switch (fgetc(f)) {
				case EOF:
					return EIO;
				case '#':
					if (read_comment(f))
						return EIO;
				break;
				case '1':
					ch = fgetc(f);
					val = 1;
					if (ch >= 0 && ch <= 5)
						val = 10 + ch - '0';
				case '0':
					run = 0;
					
					*pixel = (*pixel & 0xf0>>4*(x%2)) |
					         val<<(4*(!x%2));
				break;
				}
			}
			
			x++;
			
			if (x%2 == 0)
				pixel++;
		}

		if (context->w%2)
			pixel++;
	}

	return 0;
}

int GP_PXMLoad8bpp(FILE *f, GP_Context *context)
{
	uint8_t *pixel = context->pixels;
	uint32_t x, y;

	for (y = 0; y < context->h; y++) {
		for (x = 0; x < context->w; x++) {
			int run = 1;
			int val = 0;
			char ch;

			while (run) {
				switch (ch = fgetc(f)) {
				case EOF:
					return EIO;
				case '#':
					if (read_comment(f))
						return EIO;
				break;
				case '1' ... '9':
					val = ch - '0';
					ch  = fgetc(f);
					if (ch >= '0' && ch <= '9') {
						val = val * 10 + ch - '0';
						ch = fgetc(f);
						if (ch >= '0' && ch <= '9')
							val = val * 10 + ch - '0';
					}
					
					if (val > 255)
						return EIO;
				case '0':
					run = 0;
					
					*pixel = val;
				break;
				default:
				break;
				}
			}
			
			pixel++;
		}
	}

	return 0;
}

#define BITMASK(byte, bit) (!!((byte)&(0x80>>(bit))))

static int write_line_1bpp(FILE *f, const uint8_t *data, GP_Context *src)
{
	uint32_t x, max = src->bytes_per_row;
	int ret;

	if (src->w % 8)
		max--;

	for (x = 0; x < max; x++) {
		
		if (x != 0)
			if (fprintf(f, " ") < 0)
				return EIO;
		
		ret = fprintf(f, "%u %u %u %u %u %u %u %u",
		                 BITMASK(data[x], 0),
		                 BITMASK(data[x], 1),
		                 BITMASK(data[x], 2),
		                 BITMASK(data[x], 3),
		                 BITMASK(data[x], 4),
		                 BITMASK(data[x], 5),
		                 BITMASK(data[x], 6),
		                 BITMASK(data[x], 7));
		if (ret < 0)
			return EIO;
	}

	for (x = 0; x < (src->w % 8); x++) {
		ret = fprintf(f, " %u", BITMASK(data[max], x));

		if (ret < 0)
			return EIO;
	}

	if (fprintf(f, "\n") < 0)
		return EIO;

	return 0;
}

int GP_PXMSave1bpp(FILE *f, GP_Context *context)
{
	uint32_t y;
	int ret;

	for (y = 0; y < context->h; y++) {
		ret = write_line_1bpp(f, context->pixels + context->bytes_per_row * y,
		                      context);

		if (ret)
			return ret;
	}	

	return 0;
}

#define MASK_2BPP(byte, pix) (0x03 & (byte>>((3 - pix)<<1)))

static int write_line_2bpp(FILE *f, const uint8_t *data, GP_Context *src)
{
	uint32_t x, max = src->bytes_per_row;
	int ret;

	if (src->w % 4)
		max--;

	for (x = 0; x < max; x++) {
		
		if (x != 0)
			if (fprintf(f, " ") < 0)
				return EIO;
		
		ret = fprintf(f, "%u %u %u %u",
		                 MASK_2BPP(data[x], 0),
		                 MASK_2BPP(data[x], 1),
		                 MASK_2BPP(data[x], 2),
		                 MASK_2BPP(data[x], 3));
		if (ret < 0)
			return EIO;
	}

	for (x = 0; x < (src->w % 4); x++) {
		ret = fprintf(f, " %u", MASK_2BPP(data[max], x));

		if (ret < 0)
			return EIO;
	}

	if (fprintf(f, "\n") < 0)
		return EIO;

	return EIO;
}

int GP_PXMSave2bpp(FILE *f, GP_Context *context)
{
	uint32_t y;
	int ret;

	for (y = 0; y < context->h; y++) {
		ret = write_line_2bpp(f, context->pixels + context->bytes_per_row * y,
		                      context);

		if (ret)
			return ret;
	}	

	return EIO;
}

static int write_line_8bpp(FILE *f, const uint8_t *data, GP_Context *src)
{
	uint32_t x;
	int ret;

	for (x = 0; x < src->w; x++) {
		
		if (x != 0)
			if (fprintf(f, " ") < 0)
				return EIO;
		
		ret = fprintf(f, "%u", data[x]);
		
		if (ret < 0)
			return EIO;
	}

	if (fprintf(f, "\n") < 0)
		return EIO;

	return 0;
}

int GP_PXMSave8bpp(FILE *f, GP_Context *context)
{
	uint32_t y;
	int ret;

	for (y = 0; y < context->h; y++) {
		ret = write_line_8bpp(f, context->pixels + context->bytes_per_row * y,
		                      context);

		if (ret)
			return ret;
	}	

	return 0;
}
