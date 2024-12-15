// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#include <gfxprim.h>

#include "bogoman_debug.h"
#include "bogoman_map.h"
#include "bogoman_loader.h"
#include "bogoman_render.h"

#define ELEM_SIZE 27

static void save_png(struct bogoman_map *map, unsigned int elem_size,
                     const char *filename)
{
	gp_pixmap *pixmap;
	unsigned int rx, ry;

	rx = elem_size * map->w;
	ry = elem_size * map->h;

	pixmap = gp_pixmap_alloc(rx, ry, GP_PIXEL_RGB888);

	if (pixmap == NULL)
		return;

	struct bogoman_render render = {
		.map = map,
		.map_x_offset = 0,
		.map_y_offset = 0,
		.pixmap = pixmap,
		.map_elem_size = elem_size,
	};

	bogoman_render(&render, BOGOMAN_RENDER_ALL);

	gp_save_png(pixmap, filename, NULL);
	gp_pixmap_free(pixmap);
}

static struct gp_backend *backend;

static void event_loop(struct bogoman_render *render, gp_backend *backend)
{
	struct bogoman_map *map = render->map;
	char path[128];
	static int screenshots;
	gp_event *ev;

	while ((ev = gp_backend_ev_get(backend))) {
		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				break;

			switch (ev->val) {
			case GP_KEY_ESC:
				gp_backend_exit(backend);
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
			case GP_KEY_S:
				snprintf(path, sizeof(path),
				         "screenshot%04i.png", screenshots++);
				save_png(map, render->map_elem_size, path);
			break;
			}
			bogoman_render(render, BOGOMAN_RENDER_DIRTY);
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				render->pixmap = backend->pixmap;
				bogoman_render(render, BOGOMAN_RENDER_ALL);
			break;
			}
		break;
		case GP_EV_TMR:
			bogoman_map_timer_tick(render->map);
			bogoman_render(render, BOGOMAN_RENDER_DIRTY);
		break;
		}
	}


}

int main(int argc, char *argv[])
{
	struct bogoman_map *map;
	GP_TIMER_DECLARE(timer, 0, 300, "Refresh", NULL, NULL);
	const char *map_path = "map.txt";
	const char *backend_opts = NULL;
	int opt;

	bogoman_set_dbg_level(10);

	while ((opt = getopt(argc, argv, "b:m:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'm':
			map_path = optarg;
		break;
		case 'h':
			printf("Usage: %s [-b backend] [-m map]\n\n", argv[0]);
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	map = bogoman_load(map_path);
	if (!map) {
		fprintf(stderr, "Failed to load map");
		return 1;
	}

	bogoman_map_dump(map);

	backend = gp_backend_init(backend_opts, 0, 0, "Bogoman");

	if (!backend) {
		fprintf(stderr, "Failed to initialize backend");
		return 1;
	}

	struct bogoman_render render = {
		.map = map,
		.map_x_offset = 0,
		.map_y_offset = 0,
		.pixmap = backend->pixmap,
		.backend = backend,
		.map_elem_size = ELEM_SIZE,
	};

	bogoman_render(&render, BOGOMAN_RENDER_ALL);

	gp_backend_timer_add(backend, &timer);

	for (;;) {
		gp_backend_wait(backend);
		event_loop(&render, backend);
	}

	return 0;
}
