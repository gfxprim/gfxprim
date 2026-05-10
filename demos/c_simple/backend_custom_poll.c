// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Backend with a custom poll function.

  */

#include <stdio.h>
#include <poll.h>
#include <gfxprim.h>

static int timer_expirations;

static void redraw(gp_backend *self)
{
	gp_pixmap *pixmap = self->pixmap;
	gp_pixel white_pixel, black_pixel;

	if (!pixmap)
		return;

	black_pixel = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	white_pixel = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, pixmap);

	gp_fill(pixmap, black_pixel);
	gp_print(pixmap, NULL, gp_backend_w(self)/2, gp_backend_h(self)/2,
	         GP_ALIGN_CENTER|GP_VALIGN_CENTER, white_pixel, black_pixel,
	         "Timer expirations %i", timer_expirations);

	gp_backend_flip(self);
}

static void poll_events(gp_backend *backend)
{
	gp_event *ev;

	while ((ev = gp_backend_ev_poll(backend))) {
		gp_ev_dump(ev);

		switch (ev->type) {
		case GP_EV_KEY:
			switch (ev->val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				gp_backend_exit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev->code) {
			case GP_EV_SYS_RENDER_STOP:
				gp_backend_render_stopped(backend);
			break;
			case GP_EV_SYS_RENDER_START:
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				gp_backend_exit(backend);
				exit(0);
			break;
			}
		break;
		case GP_EV_TMR:
			timer_expirations++;
			redraw(backend);
		break;
		}
	}
}

int main(int argc, char *argv[])
{
	gp_backend *backend;
	const char *backend_opts = NULL;
	int fd, opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			gp_backend_init_help();
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = gp_backend_init(backend_opts, 100, 100, "Backend Example");
	if (!backend) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	fd = gp_backend_fd(backend);
	if (fd < 0) {
		fprintf(stderr, "Backend does not expose fd!\n");
		gp_backend_exit(backend);
		return 1;
	}

	gp_timer timer = {
		.expires = 0,
		.period = 1000,
		.id = "Periodic timer",
	};

	gp_backend_timer_start(backend, &timer);


	struct pollfd fds[1] = {
		{.fd = fd, .events = POLLIN},
	};

	for (;;) {
		int timeout = gp_backend_timer_timeout(backend);

		fprintf(stderr, "Timeout %i\n", timeout);

		poll(fds, GP_ARRAY_SIZE(fds), timeout);
		poll_events(backend);
	}

	return 0;
}
