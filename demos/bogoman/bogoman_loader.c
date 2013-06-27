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
#include <stdlib.h>
#include <string.h>

#include "bogoman_debug.h"
#include "bogoman_common.h"
#include "bogoman_loader.h"

/*
 * Counts map w and h by reading number of lines and maximal size of line till
 * first empty line or EOF.
 */
static void get_map_info(FILE *f, unsigned int *w, unsigned int *h)
{
	unsigned int curw = 0;
	int ch;

	*w = 0;
	*h = 0;

	while ((ch = getc(f)) != EOF) {
		switch (ch) {
		case '\n':
			if (*w < curw)
				*w = curw;

			if (curw == 0) {
				rewind(f);
				return;
			}

			curw = 0;
			(*h)++;
		break;
		default:
			curw++;
		}
	}

	rewind(f);
}

static enum bogoman_map_elem_id id_from_char(const char ch)
{
	switch (ch) {
	case ' ':
		return BOGOMAN_NONE;
	case '@':
		return BOGOMAN_PLAYER;
	case '-':
	case '|':
	case '+':
	case '/':
	case '\\':
		return BOGOMAN_WALL;
	case '$':
		return BOGOMAN_DIAMOND;
	case 'M':
		return BOGOMAN_MOVEABLE;
	case 'E':
		return BOGOMAN_EDIBLE;
	}

	WARN("Unknown map character '%c'", ch);

	return BOGOMAN_NONE;
}

#define LINE_MAX 256

struct line {
	unsigned int len;
	unsigned char line[LINE_MAX];
};

static void get_line(FILE *f, struct line *l)
{
	int ch;

	l->len = 0;

	while ((ch = getc(f)) != EOF) {
		switch (ch) {
		case '\n':
			return;
		default:
			l->line[l->len++] = id_from_char(ch);
		break;
		}
	}
}

static void load_map(FILE *f, struct bogoman_map *map)
{
	struct line line_a, line_b;
	struct line *line_cur, *line_next;
	unsigned int y = 0;
	unsigned int player_found = 0;

	line_cur = &line_a;
	line_next = &line_b;

	get_line(f, line_cur);
	get_line(f, line_next);

	while (line_cur->len != 0) {
		unsigned int x;

		for (x = 0; x < line_cur->len; x++) {
			struct bogoman_map_elem *elem;

			elem = bogoman_get_map_elem(map, x, y);

			elem->id = line_cur->line[x];

			switch (elem->id) {
			/* Compute wall continuations */
			case BOGOMAN_WALL:
				if (x > 0 &&
				    line_cur->line[x - 1] == BOGOMAN_WALL)
					elem->flags |= BOGOMAN_WALL_LEFT;

				if (x + 1 < line_cur->len &&
				    line_cur->line[x + 1] == BOGOMAN_WALL)
				    	elem->flags |= BOGOMAN_WALL_RIGHT;

				if (y > 0 &&
				    bogoman_map_is_id(map, x, y-1, BOGOMAN_WALL))
					elem->flags |= BOGOMAN_WALL_UP;

				if (x < line_next->len &&
				    line_next->line[x] == BOGOMAN_WALL)
					elem->flags |= BOGOMAN_WALL_DOWN;

			break;
			case BOGOMAN_PLAYER:
				if (player_found)
					WARN("Duplicated player at %ux%u previously at %ux%u\n",
					     x, y, map->player_x, map->player_y);

				map->player_x = x;
				map->player_y = y;

				player_found = 1;
			break;
			case BOGOMAN_DIAMOND:
				map->diamonds_total++;
			break;
			default:
			break;
			}
		}

		SWAP(line_cur, line_next);
		get_line(f, line_next);
		y++;
	}

	if (!player_found)
		WARN("No player found in map\n");
}

struct bogoman_map *bogoman_load(const char *path)
{
	FILE *f = fopen(path, "r");

	if (f == NULL)
		return NULL;

	unsigned int w, h;

	get_map_info(f, &w, &h);

	DEBUG(1, "Have map %ux%u\n", w, h);

	struct bogoman_map *map;
	size_t map_size;

	map_size = sizeof(struct bogoman_map) +
	           w * h * sizeof(struct bogoman_map_elem);

	map = malloc(map_size);

	if (map == NULL)
		goto err0;

	memset(map, 0, map_size);

	map->w = w;
	map->h = h;

	load_map(f, map);

	return map;
err0:
	fclose(f);
	return NULL;
}
