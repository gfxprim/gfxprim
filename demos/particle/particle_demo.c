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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Particle demo.

  */

#include <signal.h>
#include <string.h>
#include <GP.h>
#include <backends/GP_Backends.h>

#include "space.h"

static GP_Pixel black_pixel;
static GP_Pixel white_pixel;

static GP_Backend *backend = NULL;
static GP_Context *context = NULL;

static void sighandler(int signo)
{
	if (backend != NULL)
		GP_BackendExit(backend);
	
	fprintf(stderr, "Got signal %i\n", signo);

	exit(1);
}

static void init_backend(const char *backend_opts)
{
	if (!strcmp(backend_opts, "fb")) {
		fprintf(stderr, "Initalizing framebuffer backend\n");
		backend = GP_BackendLinuxFBInit("/dev/fb0");
	}
	
	if (!strcmp(backend_opts, "SDL")) {
		fprintf(stderr, "Initalizing SDL backend\n");
		backend = GP_BackendSDLInit(800, 600, 0, 0);
	}

	if (!strcmp(backend_opts, "SDL:FS")) {
		fprintf(stderr, "Initalizing SDL fullscreen\n");
		backend = GP_BackendSDLInit(0, 0, 0, GP_SDL_FULLSCREEN);
	}

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n", backend_opts);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "fb";
	int opt;
	int pause_flag = 1;

	while ((opt = getopt(argc, argv, "b:Ii:Ps:r:")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
		}
	}
	
	GP_SetDebugLevel(10);

	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGBUS, sighandler);
	signal(SIGABRT, sighandler);

	init_backend(backend_opts);

	context = backend->context;
	
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, black_pixel);
	GP_BackendFlip(backend);

	struct space *space = space_create(1000, context->w<<8, context->h<<8);

	for (;;) {
		if (backend->Poll)
			GP_BackendPoll(backend);

		usleep(5000);

		/* Read and parse events */
		GP_Event ev;

		while (GP_EventGet(&ev)) {

			GP_EventDump(&ev);
			
			switch (ev.type) {
			case GP_EV_KEY:
				if (ev.code != GP_EV_KEY_DOWN)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
				case GP_KEY_ENTER:
				case GP_KEY_Q:
					GP_BackendExit(backend);
					return 0;
				break;
				case GP_KEY_P:
					pause_flag = !pause_flag;
				break;
				}
			break;
			}
		}
	
		if (!pause_flag) {
			space_time_tick(space, 2);
			space_draw_particles(context, space);
			GP_BackendFlip(backend);
		}
	}

	GP_BackendExit(backend);

	return 0;
}
