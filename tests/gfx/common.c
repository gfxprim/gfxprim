// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>

#include "tst_test.h"
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

#define CANARY_BYTES 100

gp_pixmap *pixmap_alloc_canary(gp_size w, gp_size h, gp_pixel_type ptype)
{
	gp_pixmap *ret = gp_pixmap_alloc(w, h, ptype);

	if (!ret)
		return NULL;

	free(ret->pixels);

	size_t size = ret->bytes_per_row * h;

	ret->pixels = malloc(size + 2 * CANARY_BYTES);
	if (!ret->pixels) {
		free(ret);
		return NULL;
	}

	memset(ret->pixels, 0, size + 2 * CANARY_BYTES);
	ret->pixels += CANARY_BYTES;

	return ret;
}

int check_canary(gp_pixmap *pixmap)
{
	unsigned int i, ret = 0;
	unsigned char *sc = pixmap->pixels - CANARY_BYTES;
	unsigned char *ec = pixmap->pixels + pixmap->bytes_per_row * pixmap->h;

	for (i = 0; i < CANARY_BYTES; i++) {
		if (sc[i]) {
			tst_msg("Corrupted memmory %i bytes before pixmap", -(i - 50));
			ret = 1;
		}
	}

	for (i = 0; i < CANARY_BYTES; i++) {
		if (ec[i]) {
			tst_msg("Corrupted memmory %i bytes after pixmap", i);
			ret = 1;
		}
	}

	return ret;
}
