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

/*
 * Byte utils are utils to ease reading and parsing various image headers.
 *
 * This is internal API for loaders.
 */

#include <stdint.h>
#include <stdio.h>

#include <loaders/GP_ByteUtils.h>

#define FILENAME "file.tmp"

static void write_file(void)
{
	FILE *f;
	int ret;

	f = fopen(FILENAME, "wb");

	if (f == NULL) {
		fprintf(stderr, "Failed to open file '" FILENAME "'");
		return;
	}

	uint16_t w = 800;
	uint16_t h = 600;
	uint8_t bpp = 4;
	char *sig = "MG";

	ret = GP_FWrite(f, "%a2%x00%x00%b2%b2%b1", sig, w, h, bpp);

	if (ret != 6)
		printf("Failed to write header, ret = %i\n", ret);

	fclose(f);
}

static void read_file(void)
{
	FILE *f;
	int ret;

	f = fopen(FILENAME, "rb");

	if (f == NULL) {
		fprintf(stderr, "Failed to open file '" FILENAME "'");
		return;
	}

	uint16_t w;
	uint16_t h;
	uint8_t bpp;
	char sig[3] = {0};

	ret = GP_FRead(f, "%a2%x00%x00%b2%b2%b1", sig, &w, &h, &bpp);

	if (ret != 6)
		printf("Failed to read header, ret = %i\n", ret);

	printf("SIG=%s, w=%u, h=%u, bpp=%u\n", sig, w, h, bpp);

	fclose(f);
}

int main(void)
{
	write_file();
	read_file();
	return 0;
}
