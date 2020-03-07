// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include "bogoman_common.h"
#include "bogoman_debug.h"

#include "bogoman_map.h"

static void print_wall(struct bogoman_map_elem *elem)
{
	switch (elem->flags) {
	case BOGOMAN_LEFT:
	case BOGOMAN_RIGHT:
	case BOGOMAN_LEFT | BOGOMAN_RIGHT:
		printf("-");
	break;
	case BOGOMAN_UP:
	case BOGOMAN_DOWN:
	case BOGOMAN_UP | BOGOMAN_DOWN:
		printf("|");
	break;
	case BOGOMAN_UP | BOGOMAN_RIGHT:
	case BOGOMAN_DOWN | BOGOMAN_LEFT:
		printf("\\");
	break;
	case BOGOMAN_UP | BOGOMAN_LEFT:
	case BOGOMAN_DOWN | BOGOMAN_RIGHT:
		printf("/");
	break;
	default:
		printf("+");
	break;
	}
}

static void print_particle(struct bogoman_map_elem *elem)
{
	char c = '?';
	int dir = elem->flags & BOGOMAN_DIRECTION_MASK;

	if (elem->flags & BOGOMAN_PARTICLE_ROUND) {
		switch (dir) {
		case BOGOMAN_LEFT:
			c = '(';
		break;
		case BOGOMAN_RIGHT:
			c = ')';
		break;
		case BOGOMAN_UP:
			c = 'O';
		break;
		case BOGOMAN_DOWN:
			c = 'o';
		break;
		}
	}

	if (elem->flags & BOGOMAN_PARTICLE_SQUARE) {
		switch (dir) {
		case BOGOMAN_LEFT:
			c = '<';
		break;
		case BOGOMAN_RIGHT:
			c = '>';
		break;
		case BOGOMAN_UP:
			c = '^';
		break;
		case BOGOMAN_DOWN:
			c = 'v';
		break;
		}
	}

	putchar(c);
}

void bogoman_map_dump(struct bogoman_map *map)
{
	unsigned int x, y;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			struct bogoman_map_elem *elem;

			elem = bogoman_get_map_elem(map, x, y);

			switch (elem->id) {
			case BOGOMAN_NONE:
				printf(" ");
			break;
			case BOGOMAN_PLAYER:
				printf("@");
			break;
			case BOGOMAN_DIAMOND:
				printf("$");
			break;
			case BOGOMAN_MOVEABLE:
				printf("M");
			break;
			case BOGOMAN_EDIBLE:
				printf("E");
			break;
			case BOGOMAN_WALL:
				print_wall(elem);
			break;
			case BOGOMAN_PARTICLE:
				print_particle(elem);
			break;
			}
		}

		printf("\n");
	}

	printf("Player at %ux%u diamonds total %u\n",
	       map->player_x, map->player_y, map->diamonds_total);
}

static void copy_block(struct bogoman_map *map,
                       int dst_x, int dst_y,
                       int src_x, int src_y)
{
	struct bogoman_map_elem *src, *dst;

	src = bogoman_get_map_elem(map, src_x, src_y);
	dst = bogoman_get_map_elem(map, dst_x, dst_y);

	*dst = *src;

	dst->dirty = 1;
}

static void clear_block(struct bogoman_map *map,
                        unsigned int x, unsigned int y)
{
	struct bogoman_map_elem *block;

	block = bogoman_get_map_elem(map, x, y);

	block->id = BOGOMAN_NONE;
	block->dirty = 1;
}

/*
 * Removes diamond when player has stepped on it.
 */
static void move_get_diamond(struct bogoman_map *map,
                             unsigned int x, unsigned int y)
{
	clear_block(map, x, y);

	map->player_diamonds++;

	DEBUG(1, "Diamond taken, yet to collect %u\n",
	      map->diamonds_total - map->player_diamonds);
}

/*
 * Moves block to empty space, only one of dx or dy can be set
 * to nonzero value and the value could only be 1 or -1.
 */
static int try_move_block(struct bogoman_map *map,
                           int x, int y, int dx, int dy)
{
	int new_x = (int)x + dx;
	int new_y = (int)y + dy;

	if (bogoman_coord_in_map(map, new_x, new_y)) {

		if (!bogoman_is_empty(map, new_x, new_y))
			return 0;

		copy_block(map, new_x, new_y, x, y);
	} else {
		DEBUG(1, "Block moved out of screen %ux%u -> %ix%i",
		      x, y, new_x, new_y);
	}

	clear_block(map, x, y);
	return 1;
}

void bogoman_map_player_move(struct bogoman_map *map, int x, int y)
{
	if (x != 0 && y != 0) {
		WARN("Player can move only in one direction\n");
		return;
	}

	int new_x = map->player_x + x;
	int new_y = map->player_y + y;

	if (new_x < 0 || new_x >= (int)map->w ||
	    new_y < 0 || new_y >= (int)map->h) {
		WARN("Player can move only in screen got %ix%i\n",
		     new_x, new_y);
		return;
	}

	int player_x = map->player_x;
	int player_y = map->player_y;

	DEBUG(1, "Player %ix%i -> %ix%i\n", player_x, player_y, new_x, new_y);

	while (player_x != new_x || player_y != new_y) {
		int dx = SIGN(new_x - player_x);
		int dy = SIGN(new_y - player_y);

		int px = map->player_x + dx;
		int py = map->player_y + dy;

		switch (bogoman_get_map_elem_id(map, px, py)) {
		case BOGOMAN_NONE:
		break;
		case BOGOMAN_DIAMOND:
			move_get_diamond(map, px, py);
		break;
		case BOGOMAN_PARTICLE:
		case BOGOMAN_MOVEABLE:
			if (!try_move_block(map, px, py, dx, dy))
				goto finish_move;
		break;
		case BOGOMAN_EDIBLE:
			clear_block(map, px, py);
		break;
		default:
			goto finish_move;
		}

		player_x = px;
		player_y = py;
	}

	/* Update the map */
	struct bogoman_map_elem *player, *space;

finish_move:

	player = bogoman_get_map_elem(map, map->player_x, map->player_y);
	space = bogoman_get_map_elem(map, player_x, player_y);

	*player = map->under_player;
	map->under_player = *space;
	space->id = BOGOMAN_PLAYER;

	map->player_x = player_x;
	map->player_y = player_y;

	/* turn on dirty flags */
	player->dirty = 1;
	space->dirty = 1;
}

void bogoman_map_timer_tick(struct bogoman_map *map)
{
	unsigned int x, y, moved;
	struct bogoman_map_elem *elem;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			struct bogoman_map_elem *elem;
			elem = bogoman_get_map_elem(map, x, y);
			elem->moved = 0;
		}
	}

	do {
		moved = 0;

		for (y = 0; y < map->h; y++) {
			for (x = 0; x < map->w; x++) {
				int dir_x = 0, dir_y = 0;
				elem = bogoman_get_map_elem(map, x, y);

				if (elem->id != BOGOMAN_PARTICLE)
					continue;

				if (elem->moved)
					continue;

				switch (elem->flags & BOGOMAN_DIRECTION_MASK) {
				case BOGOMAN_LEFT:
					dir_x = -1;
				break;
				case BOGOMAN_RIGHT:
					dir_x = 1;
				break;
				case BOGOMAN_UP:
					dir_y = -1;
				break;
				case BOGOMAN_DOWN:
					dir_y = +1;
				break;
				default:
					continue;
				}

				if (!bogoman_is_empty(map, x + dir_x, y + dir_y)) {
					if (!(elem->flags & BOGOMAN_PARTICLE_ROUND))
						continue;

					if (dir_x) {
						//TODO randomly choose direction
						if (bogoman_is_empty(map, x+dir_x, y-1))
							dir_y = -1;
						if (bogoman_is_empty(map, x+dir_x, y+1))
							dir_y = 1;
					}

					if (dir_y) {
						if (bogoman_is_empty(map, x-1, y+dir_y))
							dir_x = -1;
						if (bogoman_is_empty(map, x+1, y+dir_y))
							dir_x = 1;
					}

					if (!dir_x || !dir_y)
						continue;
				}

				elem->moved = 1;
				bogoman_switch(map, x, y, x + dir_x, y + dir_y);
				moved++;
			}
		}
	} while (moved);
}
