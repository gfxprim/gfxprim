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

#include <GP.h>

#include "bogoman_debug.h"
#include "bogoman_map.h"
#include "bogoman_loader.h"
#include "bogoman_render.h"

#define ELEM_SIZE 33

static void save_png(struct bogoman_map *map, unsigned int elem_size,
                     const char *filename)
{
	GP_Context *ctx;
	unsigned int rx, ry;

	rx = elem_size * map->w;
	ry = elem_size * map->h;

	ctx = GP_ContextAlloc(rx, ry, GP_PIXEL_RGB888);

	if (ctx == NULL)
		return;
	
	struct bogoman_render render = {
		.map = map,
		.map_x_offset = 0,
		.map_y_offset = 0,
		.ctx = ctx,
		.map_elem_size = elem_size,
	};
	
	bogoman_render(&render, BOGOMAN_RENDER_ALL);

	GP_SavePNG(ctx, filename, NULL);
	GP_ContextFree(ctx);
}
	
static struct GP_Backend *backend;

static void event_loop(struct bogoman_map *map)
{
	while (GP_EventsQueued()) {
		GP_Event ev;
		
		GP_EventGet(&ev);

		switch (ev.type) {
		case GP_EV_KEY:
			if (ev.code != GP_EV_KEY_DOWN)
				break;

			switch (ev.val.val) {
			case GP_KEY_ESC:
				GP_BackendExit(backend);
				exit(0);
			break;
			case GP_KEY_RIGHT:
				bogoman_map_player_move(map, 1, 0);
			break;
			case GP_KEY_LEFT:
				bogoman_map_player_move(map, -1, 0);
			break;
			case GP_KEY_UP:
				bogoman_map_player_move(map, 0, -1);
			break;
			case GP_KEY_DOWN:
				bogoman_map_player_move(map, 0, 1);
			break;
			}
		break;
		}
	}


}

int main(int argc, char *argv[])
{
	struct bogoman_map *map;

	bogoman_set_dbg_level(10);

	if (argc > 1)
		map = bogoman_load(argv[1]);
	else
		map = bogoman_load("map.txt");

	if (map == NULL) {
		fprintf(stderr, "Failed to load map");
		return 1;
	}

	bogoman_map_dump(map);

	unsigned int cw = map->w * ELEM_SIZE;
	unsigned int ch = map->h * ELEM_SIZE;

	backend = GP_BackendX11Init(NULL, 0, 0, cw, ch, "Bogoman", 0);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initialize backend");
		return 1;
	}

	struct bogoman_render render = {
		.map = map,
		.map_x_offset = 0,
		.map_y_offset = 0,
		.ctx = backend->context,
		.map_elem_size = ELEM_SIZE,
	};

	bogoman_render(&render, BOGOMAN_RENDER_ALL);
	GP_BackendFlip(backend);

	for (;;) {
		GP_BackendPoll(backend);
		event_loop(map);
		
		bogoman_render(&render, BOGOMAN_RENDER_DIRTY);
		GP_BackendFlip(backend);
	
		usleep(100000);
	}

	return 0;
}
