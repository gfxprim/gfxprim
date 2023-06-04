//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <stdlib.h>

#include <utils/gp_markup_justify.h>

static size_t word_len(gp_markup_glyph *first)
{
	size_t len = 0;

	for (;;) {
		switch (first[len].glyph) {
		case ' ':
		case '\n':
		case 0:
			return len;
		}
		len++;
	}
}

#include <stdio.h>

static void linebreak(unsigned int *cnt, gp_markup_lines *res, gp_markup_glyph *first, gp_markup_glyph *last)
{
	(*cnt)++;

	if (!res)
		return;

	res->lines[res->lines_cnt].first = first;
	res->lines[res->lines_cnt].last = last;
	res->lines_cnt++;
}

static unsigned int justify_greedy(gp_markup *self, unsigned int line_width,
                                   gp_markup_width_cb width_callback, void *priv, gp_markup_lines *res)
{
	unsigned int cnt = 0;
	unsigned int cur_width = 0;
	gp_markup_glyph *i = self->glyphs;
	gp_markup_glyph *first = i, *last = i;

	while (i->glyph) {
		size_t len = word_len(i);
		unsigned int width = width_callback(i, len, priv);

		if (cur_width + width > line_width) {
			if (cur_width) {
				linebreak(&cnt, res, first, last);
				first = i;
				cur_width = 0;
			} else {
				/* Break a "word" somehow */
				size_t l;
				width = 0;
				for (l = 0; l < len; l++) {
					width += width_callback(i+l, 1, priv);
					if (width > line_width) {
						linebreak(&cnt, res, first, i + l - 1);
						first = i + l;
						width = 0;
					}
				}
				cur_width = 0;
				i = first;
			}
		} else {
			cur_width += width;
			i += len;
		}

		last = i-1;

		int set_first = 0;

		for (;;) {
			if (i->glyph ==  ' ') {
				if (cur_width)
					cur_width += width_callback(i, 1, priv);
			} else if (i->glyph == '\n') {
				if (cur_width) {
					linebreak(&cnt, res, first, i-1);

					if (i->fmt & GP_MARKUP_STRIKE)
						linebreak(&cnt, res, NULL, i);
				} else {
					linebreak(&cnt, res, NULL, i);
				}


				set_first = 1;
				cur_width = 0;
			} else {
				if (set_first)
					first = i;
				break;
			}

			i++;
		}
	}

	if (cur_width)
		linebreak(&cnt, res, first, i-1);

	return cnt;
}

gp_markup_lines *gp_markup_justify(gp_markup *self, unsigned int line_width,
                                   gp_markup_width_cb width_callback, void *priv)
{
	unsigned int lines_cnt;

	lines_cnt = justify_greedy(self, line_width, width_callback, priv, NULL);

	gp_markup_lines *ret = malloc(sizeof(gp_markup_lines) + sizeof(gp_markup_line) * lines_cnt);
	if (!ret)
		return NULL;

	ret->lines_cnt = 0;

	justify_greedy(self, line_width, width_callback, priv, ret);

	return ret;
}

void gp_markup_justify_free(gp_markup_lines *self)
{
	free(self);
}

void gp_markup_justify_dump(gp_markup_lines *self)
{
	unsigned int i;

	for (i = 0; i < self->lines_cnt; i++) {
		gp_markup_glyph *g = self->lines[i].first;

		printf("|");

		for (;;) {
			if (g)
				printf("%c", g->glyph);

			if (g == self->lines[i].last)
				break;

			if (g)
				g++;
			else
				g = self->lines[i].last;
		}

		printf("|\n");
	}
}
