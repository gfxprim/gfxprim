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

#include <stdio.h>

#include "common.h"

static void dump_buffer(const char *pattern, int w, int h)
{
	int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++)
			printf("%2x ", (uint8_t)pattern[x + y * w]);
		printf("\n");
	}
}

void dump_buffers(const char *pattern, const gp_pixmap *c)
{
	printf("Expected pattern:\n");
	dump_buffer(pattern, c->w, c->h);
	printf("Rendered pattern:\n");
	dump_buffer((char*)c->pixels, c->w, c->h);
	printf("Difference:\n");

	unsigned int x, y;

	for (y = 0; y < c->h; y++) {
		for (x = 0; x < c->w; x++) {
			unsigned int idx = x + y * c->w;
			char p = ((char*)c->pixels)[idx];

			if (pattern[idx] != p) {
				/* TODO: we expect background to be 0 */
				if (p == 0)
					printf(" x ");
				else
					printf(" * ");
			} else {
				printf("%2x ", (uint8_t)pattern[idx]);
			}

		}

		printf("\n");
	}
}

int compare_buffers(const char *pattern, const gp_pixmap *c)
{
	gp_size x, y;
	int err = 0;

	for (x = 0; x < c->w; x++) {
		for (y = 0; y < c->h; y++) {
			unsigned int idx = x + y * c->w;

			if (pattern[idx] != ((char*)c->pixels)[idx])
				err++;
		}
	}

	if (err)
		dump_buffers(pattern, c);

	return err;
}
