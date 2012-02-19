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

#include <string.h>

#include "core/GP_Debug.h"

#include "backends/GP_LinuxFB.h"
#include "backends/GP_SDL.h"
#include "backends/GP_BackendInit.h"

static void backend_sdl_help(FILE *help, const char *err)
{
	if (help == NULL)
		return;

	if (err != NULL)
		fprintf(help, "ERROR: %s\n", err);

	fprintf(help, "libSDL backend\n"
	              "--------------\n"
	              "SDL:[FS]\n");
}

static GP_Backend *backend_sdl_init(const char *params, FILE *help)
{
	if (params == NULL)
		return GP_BackendSDLInit(0, 0, 0, 0);

	if (!strcasecmp(params, "FS"))
		return GP_BackendSDLInit(0, 0, 0, GP_SDL_FULLSCREEN);

	backend_sdl_help(help, "SDL: Invalid parameters");

	return NULL;
}

static void backend_fb_help(FILE *help, const char *err)
{
	if (help == NULL)
		return;

	if (err != NULL)
		fprintf(help, "ERROR: %s\n", err);

	fprintf(help, "LinuxFB backend\n"
	              "--------------\n"
	              "FB:[/dev/fbX]\n");
}

static GP_Backend *backend_fb_init(const char *params, FILE *help)
{
	const char *fb = "/dev/fb0";
	
	(void) help;

	if (params != NULL)
		fb = params;

	return GP_BackendLinuxFBInit(fb);
}

static const char *backend_names[] = {
	"SDL", /* libSDL            */
	"FB",  /* Linux Framebuffer */
	NULL,
};

static GP_Backend *(*backend_inits[])(const char *params, FILE *help) = {
	backend_sdl_init,
	backend_fb_init,
	NULL,
};

static void (*backend_helps[])(FILE *help, const char *err) = {
	backend_sdl_help,
	backend_fb_help,
	NULL,
};

static void print_help(FILE *help, char *err)
{
	int i;

	if (help == NULL)
		return;

	if (err != NULL) {
		fprintf(help, "ERROR: %s\n", err);
		fprintf(help, "\n");
	}

	fprintf(help, "Backends usage\n"
	              "--------------\n\n");

	for (i = 0; backend_helps[i] != NULL; i++) {
		backend_helps[i](help, NULL);
		fprintf(help, "\n");
	}
}

static int get_backend(const char *name)
{
	int i;

	for (i = 0; backend_names[i] != 0; i++)
		if (!strcasecmp(name, backend_names[i]))
			return i;
	
	return -1;
}

static GP_Backend *init_backend(const char *name, const char *params, FILE *help)
{
	int i = get_backend(name);

	if (i < 0) {
		GP_DEBUG(1, "Invalid backend name '%s'", name);
		print_help(help, "Invalid backend name");
		return NULL;
	}

	return backend_inits[i](params, help);
}

GP_Backend *GP_BackendInit(const char *params, FILE *help)
{
	if (params == NULL) {
		print_help(help, NULL);
		return NULL;
	}
	
	/* parse backend name */
	int i, len = strlen(params);
	char buf[len+1], *backend_params = NULL;
	
	strcpy(buf, params);

	for (i = 0; i < len; i++) {
		if (buf[i] == ':') {
			buf[i] = '\0';
			backend_params = buf + i + 1;
			break;
		}
	}

	GP_DEBUG(1, "Have backend name '%s'", buf);

	return init_backend(buf, backend_params, help);
}
