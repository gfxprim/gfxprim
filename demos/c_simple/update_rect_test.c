// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2025-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <gfxprim.h>

static gp_pixel black;
static gp_pixel white;

static gp_backend *backend;

static unsigned int x_off;
static unsigned int y_off;
static unsigned int w_off;
static unsigned int h_off;

void redraw_screen(void)
{
	gp_pixmap *pix = backend->pixmap;

	if (!pix)
		return;

	gp_fill(pix, black);
	gp_backend_update(backend);

	gp_size w = gp_pixmap_w(pix) - x_off - w_off;
	gp_size h = gp_pixmap_h(pix) - y_off - h_off;

	gp_fill_rect_xywh(pix, x_off, y_off, w, h, white);
	gp_backend_update_rect_xywh(backend, x_off, y_off, w, h);
}

static void init_colors(gp_pixel_type pixel_type)
{
	black = gp_rgb_to_pixel(0x00, 0x00, 0x00, pixel_type);
	white = gp_rgb_to_pixel(0xff, 0xff, 0xff, pixel_type);
}

void event_loop(void)
{
	gp_event *ev;

	while ((ev = gp_backend_ev_wait(backend))) {
		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->key.key) {
			case GP_KEY_U:
				gp_backend_update(backend);
			break;
			case GP_KEY_X:
				x_off++;
				redraw_screen();
			break;
			case GP_KEY_Y:
				y_off++;
				redraw_screen();
			break;
			case GP_KEY_W:
				w_off++;
				redraw_screen();
			break;
			case GP_KEY_H:
				h_off++;
				redraw_screen();
			break;
			case GP_KEY_ESC:
				gp_backend_exit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch(ev->code) {
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				exit(0);
			break;
			case GP_EV_SYS_RENDER_STOP:
				gp_backend_render_stopped(backend);
			break;
			case GP_EV_SYS_RENDER_START:
				redraw_screen();
			break;
			case GP_EV_SYS_RENDER_PIXEL_TYPE:
				init_colors(ev->pixel_type);
			break;
			}
		break;
		}
	}
}

void print_instructions(void)
{
	printf("Use the following keys to control the test:\n");
	printf("    Esc ............. exit\n");
	printf("    X ............... increase x offset\n");
	printf("    Y ............... increase y offset\n");
	printf("    W ............... decrease width\n");
	printf("    H ............... decrease height\n");
}

int main(int argc, char *argv[])
{
	int opt;
	char *backend_opts = NULL;

	print_instructions();

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 0, 0, "Update Rect Test");
	if (!backend) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	event_loop();
}
