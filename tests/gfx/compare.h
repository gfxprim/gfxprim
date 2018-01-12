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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef COMPARE_H__
#define COMPARE_H__

#include <stdio.h>
#include <core/GP_GetPutPixel.h>

static inline void print_diff(gp_pixmap *p1, gp_pixmap *p2)
{
	gp_size x, y;

	printf(" ");
	for (x = 0; x < p1->w; x++)
		printf("-");
	printf("\n");

	for (y = 0; y < p1->h; y++) {
		printf("|");
		for (x = 0; x < p1->w; x++) {
			gp_pixel px1 = gp_getpixel_raw_8BPP(p1, x, y);
			gp_pixel px2 = gp_getpixel_raw_8BPP(p2, x, y);

			if (px1 == px2) {
				if (px1)
					printf("*");
				else
					printf(" ");
			} else {
				if (px1)
					printf("1");
				else
					printf("2");
			}
		}
		printf("|\n");
	}

	printf(" ");
	for (x = 0; x < p1->w; x++)
		printf("-");
	printf("\n");
}

static inline void print(gp_pixmap *p)
{
	gp_size x, y;

	printf(" ");
	for (x = 0; x < p->w; x++)
		printf("-");
	printf("\n");

	for (y = 0; y < p->h; y++) {
		printf("|");
		for (x = 0; x < p->w; x++) {
			gp_pixel px = gp_getpixel_raw_8BPP(p, x, y);

			if (px)
				printf("*");
			else
				printf(" ");
		}
		printf("|\n");
	}

	printf(" ");
	for (x = 0; x < p->w; x++)
		printf("-");
	printf("\n");
}

static inline int compare_pixmaps(gp_pixmap *p1, gp_pixmap *p2)
{
	gp_size x, y;
	print(p1);
	for (x = 0; x < p1->w; x++) {
		for (y = 0; y < p1->h; y++) {
			gp_pixel px1 = gp_getpixel_raw_8BPP(p1, x, y);
			gp_pixel px2 = gp_getpixel_raw_8BPP(p2, x, y);

			if (px1 != px2) {
				print(p1);
				print(p2);
				print_diff(p1, p2);
				return TST_FAILED;
			}
		}
	}

	return TST_SUCCESS;
}

#endif /* COMPARE_H__ */
