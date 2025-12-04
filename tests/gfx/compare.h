// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TESTS_GFX_COMPARE_H
#define TESTS_GFX_COMPARE_H

#include <stdio.h>
#include <core/gp_get_put_pixel.h>

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

	return TST_PASSED;
}

#endif /* TESTS_GFX_COMPARE_H */
