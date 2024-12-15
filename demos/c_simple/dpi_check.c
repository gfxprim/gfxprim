// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gfxprim.h>

static gp_backend *backend;

static gp_pixel white_pixel, gray_pixel, dark_gray_pixel, black_pixel,
		red_pixel, blue_pixel;

static gp_text_style style = GP_DEFAULT_TEXT_STYLE;

static void redraw_screen(void)
{
	gp_coord w = gp_pixmap_w(backend->pixmap);
	gp_coord cx = w/2;
	gp_coord h = gp_pixmap_h(backend->pixmap);
	gp_coord cy = h/2;

	gp_fill(backend->pixmap, white_pixel);

	style.pixel_xmul = 4;
	style.pixel_ymul = 4;

	if (!backend->dpi) {
		gp_print(backend->pixmap, &style, cx, cy,
		         GP_ALIGN_CENTER | GP_VALIGN_CENTER,
			 black_pixel, white_pixel, "DPI not exported by the backend");
		return;
	}

	style.pixel_xmul = 2;
	style.pixel_ymul = 2;

	gp_print(backend->pixmap, &style, 10, 10,
	         GP_ALIGN_RIGHT | GP_VALIGN_BELOW,
		 black_pixel, white_pixel, "DPI %u", backend->dpi);

	gp_hline_xyw(backend->pixmap, 0, cy, w, black_pixel);
	gp_vline_xyh(backend->pixmap, cx, 0, h, black_pixel);

	float pixels_in_mm = gp_dpi_to_ppmm(backend->dpi);
	float x, y;
	int cnt = 0;

	for (x = 0; x < w; x += pixels_in_mm) {
		gp_size len;
		int is_cm = (cnt % 10 == 0);

		switch (cnt++ % 10) {
		case 0:
			len = 3 * pixels_in_mm;
		break;
		case 5:
			len = 2 * pixels_in_mm;
		break;
		default:
			len = pixels_in_mm;
		break;
		}

		gp_vline_xyh(backend->pixmap, cx + (x + 0.5), cy-len, len, black_pixel);
		gp_vline_xyh(backend->pixmap, cx - (x - 0.5), cy-len, len, black_pixel);

		if (is_cm && cnt/10) {
			gp_print(backend->pixmap, &style, cx + (x + 0.5), cy - 2 * len,
			         GP_ALIGN_CENTER | GP_VALIGN_CENTER,
				 black_pixel, white_pixel, "%i", cnt/10);
		}
	}

	cnt = 0;

	for (y = 0; y < h; y += pixels_in_mm) {
		gp_size len;
		int is_cm = (cnt % 10 == 0);

		switch (cnt++ % 10) {
		case 0:
			len = 3 * pixels_in_mm;
		break;
		case 5:
			len = 2 * pixels_in_mm;
		break;
		default:
			len = pixels_in_mm;
		break;
		}

		gp_hline_xyw(backend->pixmap, cx-len, cy + (y + 0.5), len, black_pixel);
		gp_hline_xyw(backend->pixmap, cx-len, cy - (y - 0.5), len, black_pixel);

		if (is_cm && cnt/10) {
			gp_print(backend->pixmap, &style, cx - 2 * len, cy - (y - 0.5),
			         GP_ALIGN_CENTER | GP_VALIGN_CENTER,
				 black_pixel, white_pixel, "%i", cnt/10);
		}
	}
}

static void event_loop(void)
{
	for (;;) {
		gp_event *ev = gp_backend_ev_wait(backend);

		switch (ev->type) {
		case GP_EV_KEY:
			if (ev->code != GP_EV_KEY_DOWN)
				continue;

			switch (ev->key.key) {
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
			case GP_EV_SYS_RESIZE:
				gp_backend_resize_ack(backend);
				redraw_screen();
				gp_backend_flip(backend);
			break;
			}
		break;
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "b:f:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			printf("\nUsage: %s [-b backend] [-f font_face]\n\n", argv[0]);
			gp_backend_init_help();
			return 0;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 800, 600, "DPI test");
	if (!backend) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}

	white_pixel     = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, backend->pixmap);
	gray_pixel      = gp_rgb_to_pixmap_pixel(0xbe, 0xbe, 0xbe, backend->pixmap);
	dark_gray_pixel = gp_rgb_to_pixmap_pixel(0x7f, 0x7f, 0x7f, backend->pixmap);
	black_pixel     = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, backend->pixmap);
	red_pixel       = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, backend->pixmap);
	blue_pixel      = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xff, backend->pixmap);

	redraw_screen();
	gp_backend_flip(backend);

	event_loop();

	return 0;
}
