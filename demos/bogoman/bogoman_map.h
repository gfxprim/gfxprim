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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef __BOGOMAN_MAP_H__
#define __BOGOMAN_MAP_H__

enum bogoman_map_elem_id {
	/* empty walkable space */
	BOGOMAN_NONE = 0x00,
	/* player */
	BOGOMAN_PLAYER = 0x01,
	/* wall */
	BOGOMAN_WALL = 0x02,
	/* diamons to collect */
	BOGOMAN_DIAMOND = 0x03,
	/* like a wall but player moveable */
	BOGOMAN_MOVEABLE = 0x04,
	/* like a diamond but doesn't counts to points */
	BOGOMAN_EDIBLE = 0x05,
	/* particles, move in defined direction unless stoppend by objects */
	BOGOMAN_PARTICLE = 0x06,

	BOGOMAN_MAX = BOGOMAN_PARTICLE,
};

/*
 * Lower 8 bits defines continuation for walls, direction for particles, etc.
 */
enum bogoman_direction_flags {
	BOGOMAN_LEFT  = 0x01,
	BOGOMAN_RIGHT = 0x02,
	BOGOMAN_UP    = 0x04,
	BOGOMAN_DOWN  = 0x08,

	BOGOMAN_DIRECTION_MASK = 0x0f,
};

enum bogoman_particle_flags {
	BOGOMAN_PARTICLE_ROUND = 0x10,
	BOGOMAN_PARTICLE_SQUARE = 0x20,
};

struct bogoman_map_elem {
	unsigned char id;
	unsigned char flags;

	/* the element changed, needs to be redrawn */
	unsigned char dirty:1;
	unsigned char moved:1;
};

struct bogoman_map {
	unsigned int w;
	unsigned int h;

	unsigned int diamonds_total;

	/* player data */
	unsigned int player_x;
	unsigned int player_y;
	unsigned int player_diamonds;
	/* used to save element player steps on */
	struct bogoman_map_elem under_player;

	struct bogoman_map_elem map[];
};

static inline struct bogoman_map_elem *
	bogoman_get_map_elem(struct bogoman_map *map,
	                     unsigned int x, unsigned int y)
{
	return &(map->map[x + y * map->w]);
}

static inline enum bogoman_map_elem_id
	bogoman_get_map_elem_id(struct bogoman_map *map,
	                        unsigned int x, unsigned int y)
{
	struct bogoman_map_elem *elem = bogoman_get_map_elem(map, x, y);

	return elem->id;
}

static inline enum bogoman_map_elem_id
	bogoman_map_elem_id(struct bogoman_map *map,
                            unsigned int x, unsigned int y)
{
	struct bogoman_map_elem *elem = bogoman_get_map_elem(map, x, y);

	return elem->id;
}

static inline int bogoman_coord_in_map(struct bogoman_map *map, int x, int y)
{
	return (x >= 0) && ((unsigned)x < map->w) &&
	       (y >= 0) && ((unsigned)y < map->w);
}

static inline int bogoman_is_empty(struct bogoman_map *map, int x, int y)
{
	if (!bogoman_coord_in_map(map, x, y))
		return 0;

	return bogoman_get_map_elem_id(map, x, y) == BOGOMAN_NONE;
}

static inline void bogoman_switch(struct bogoman_map *map,
                                  int x1, int y1, int x2, int y2)
{
	struct bogoman_map_elem *i, *j, tmp;

	i = bogoman_get_map_elem(map, x1, y1);
	j = bogoman_get_map_elem(map, x2, y2);

	tmp = *i;
	*i = *j;
	*j = tmp;

	i->dirty = 1;
	j->dirty = 1;
}

void bogoman_map_player_move(struct bogoman_map *map, int x, int y);

void bogoman_map_timer_tick(struct bogoman_map *map);

/*
 * Dumps map into the stdout.
 */
void bogoman_map_dump(struct bogoman_map *map);

#endif /* __BOGOMAN_MAP_H__ */
