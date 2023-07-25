// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#include <string.h>
#include <errno.h>

#include <core/gp_debug.h>

#include <backends/gp_backends.h>
#include <backends/gp_backend_init.h>

static char *next_param(char *params)
{
	for (;;) {
		switch (*params) {
		case ':':
			*params = '\0';
			return params + 1;
		break;
		case '\0':
			return NULL;
		break;
		}
		params++;
	}
}

#ifdef HAVE_LIBX11
static int parse_x11_params(char *params, gp_size *w, gp_size *h,
                            enum gp_x11_flags *flags)
{
	char *param;

	if (!params)
		return 0;

	do {
		param = params;
		params = next_param(params);

		if (!strcasecmp(param, "use_root")) {
			*flags |= GP_X11_USE_ROOT_WIN;
			GP_DEBUG(1, "X11: Using root window");
			continue;
		}

		if (!strcasecmp(param, "create_root")) {
			*flags |= GP_X11_CREATE_ROOT_WIN;
			GP_DEBUG(1, "X11: Creating root window");
			continue;
		}

		if (!strcasecmp(param, "disable_shm")) {
			*flags |= GP_X11_DISABLE_SHM;
			GP_DEBUG(1, "X11: Disabling SHM");
			continue;
		}

		if (!strcasecmp(param, "fs")) {
			*flags |= GP_X11_FULLSCREEN;
			GP_DEBUG(1, "X11: Enabling fullscreen");
			continue;
		}

		/*
		 * Accepts only string with format "intxint" or "intXint"
		 */
		int sw, sh;
		unsigned int n;

		if (sscanf(param, "%i%*[xX]%i%n", &sw, &sh, &n) == 2 && n == strlen(param)) {
			*w = sw;
			*h = sh;
			continue;
		}

		GP_WARN("X11: Invalid parameters '%s'", param);
		errno = EINVAL;
		return 1;
	} while (params);

	return 0;
}

static gp_backend *x11_init(char *params, const char *caption)
{
	gp_size w = 640, h = 480;
	enum gp_x11_flags flags = 0;

	if (parse_x11_params(params, &w, &h, &flags))
		return NULL;

	return gp_x11_init(NULL, 0, 0, w, h, caption, flags);
}
#endif

#ifdef HAVE_WAYLAND
static gp_backend *wayland_init(char *params, const char *caption)
{
	gp_size w = 640, h = 480;

	(void) params;

	return gp_wayland_init(NULL, w, h, caption);
}
#endif

#ifdef HAVE_LIBSDL
static int parse_sdl_params(char *params, gp_size *w, gp_size *h,
                            gp_size *bpp, uint8_t *flags)
{
	char *param;

	if (!params)
		return 0;

	do {
		param = params;
		params = next_param(params);

		if (!strcasecmp(param, "FS")) {
			*flags |= GP_SDL_FULLSCREEN;
			GP_DEBUG(1, "SDL fullscreen enabled");
			continue;
		}

		if (!strcmp(param, "8")) {
			*bpp = 8;
			GP_DEBUG(1, "SDL depth set to 8");
			continue;
		}

		if (!strcmp(param, "16")) {
			*bpp = 16;
			GP_DEBUG(1, "SDL depth set to 16");
			continue;
		}

		if (!strcmp(param, "24")) {
			*bpp = 24;
			GP_DEBUG(1, "SDL depth set to 24");
			continue;
		}

		if (!strcmp(param, "32")) {
			*bpp = 32;
			GP_DEBUG(1, "SDL depth set to 32");
			continue;
		}

		/*
		 * Accepts only string with format "intxint" or "intXint"
		 */
		int sw, sh;
		unsigned int n;

		if (sscanf(param, "%i%*[xX]%i%n", &sw, &sh, &n) == 2 && n == strlen(param)) {
			*w = sw;
			*h = sh;
			continue;
		}

		GP_WARN("SDL: Invalid parameters '%s'", param);
		errno = EINVAL;
		return 1;
	} while (params);

	return 0;
}

static gp_backend *sdl_init(char *params, const char *caption)
{
	gp_size w = 640, h = 480, bpp = 0;
	uint8_t flags = GP_SDL_RESIZABLE;

	if (parse_sdl_params(params, &w, &h, &bpp, &flags))
		return NULL;

	return gp_sdl_init(w, h, bpp, flags, caption);
}
#endif

#ifdef OS_LINUX
static int parse_fb_params(char *params, int *flags, const char **fb)
{
	char *param;

	if (!params)
		return 0;

	do {
		param = params;
		params = next_param(params);

		if (!strcasecmp(param, "no_shadow")) {
			*flags &= ~GP_FB_SHADOW;
			GP_DEBUG(1, "Shadow framebuffer disabled");
			continue;
		}

		if (!strcasecmp(param, "new_console")) {
			*flags |= GP_FB_ALLOC_CON;
			GP_DEBUG(1, "Console allocation enabled");
			continue;
		}

		*fb = param;

		if (strncmp(*fb, "/dev/", 5))
			GP_WARN("Console dev set to '%s', are you sure?", *fb);

		GP_DEBUG(1, "Framebuffer console set to '%s'", *fb);

	} while (params);

	return 0;
}

static gp_backend *fb_init(char *params, const char *caption)
{
	const char *fb = "/dev/fb0";

	(void) caption;

	int flags = GP_FB_INPUT_KBD | GP_FB_SHADOW;

	parse_fb_params(params, &flags, &fb);

	return gp_linux_fb_init(fb, flags);
}
#endif

#ifdef HAVE_AALIB
static gp_backend *aa_init(char *params, const char *caption)
{
	(void) caption;
	(void) params;

	return gp_aalib_init();
}
#endif

#ifdef HAVE_LIBXCB
static int parse_xcb_params(char *params, gp_size *w, gp_size *h)
{
	char *param;

	if (!params)
		return 0;

	do {
		param = params;
		params = next_param(params);

		/*
		 * Accepts only string with format "intxint" or "intXint"
		 */
		int sw, sh;
		unsigned int n;

		if (sscanf(param, "%i%*[xX]%i%n", &sw, &sh, &n) == 2 && n == strlen(param)) {
			*w = sw;
			*h = sh;
			continue;
		}

		GP_WARN("XCB: Invalid parameters '%s'", param);
		errno = EINVAL;
		return 1;
	} while (params);

	return 0;
}

static gp_backend *xcb_init(char *params, const char *caption)
{
	gp_size w = 640, h = 480;

	if (parse_xcb_params(params, &w, &h))
		return NULL;

	return gp_xcb_init(NULL, 0, 0, w, h, caption);
}
#endif

#ifdef OS_LINUX
static gp_backend *proxy_init(char *params, const char *caption)
{
	return gp_proxy_init(params, caption);
}
#endif

static gp_backend *init_backend(const char *name, char *params,
                                const char *caption);

static gp_backend *virt_init(char *params, const char *caption)
{
	char *pixel, *backend_name;
	gp_backend *backend;
	gp_pixel_type pixel_type;

	if (!params)
		return NULL;

	pixel = params;
	params = next_param(params);

	pixel_type = gp_pixel_type_by_name(pixel);

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_WARN("Invalid pixel type '%s'", pixel);
		return NULL;
	}

	if (!params) {
		GP_WARN("Missing backend");
		return NULL;
	}

	backend_name = params;
	params = next_param(params);

	backend = init_backend(backend_name, params, caption);
	if (!backend)
		return NULL;

	return gp_backend_virt_init(backend, pixel_type, GP_BACKEND_CALL_EXIT);
}

struct backend_init {
	const char *name;
	gp_backend *(*init)(char *params, const char *caption);
	const char *usage;
	const char *help[10];
};

static gp_backend *do_help(char *params, const char *caption);

static struct backend_init backends[] = {
#ifdef OS_LINUX
	{.name = "proxy",
	 .init = proxy_init,
	 .usage = "path",
	 .help = {"path - Path to an UNIX socket"}
	},
#endif
#ifdef HAVE_LIBXCB
	{.name  = "XCB",
	 .init = xcb_init,
	 .usage = "",
	 .help = {NULL},
	},
#endif
#ifdef HAVE_WAYLAND
	{.name = "wayland",
	 .init = wayland_init,
	 .usage = "wayland",
	 .help = {"", NULL}
	},
#endif
#ifdef HAVE_LIBX11
	{.name  = "X11",
	 .init  = x11_init,
	 .usage = "X11:[WxH]:[use_root]:[create_root]:[disable_shm]",
	 .help  = {"use_root    - starts the backend in the root window",
	           "              (w and h, if set, are ignored)",
	           "create_root - starts the backend in newly created",
	           "              root window (w and h, if set, are ignored)",
	           "disable_shm - disable MIT SHM even if available",
	           "fs          - start fullscreen",
	           NULL}
	},
#endif
	{
	 .name = "virt",
	 .init = virt_init,
	 .usage = "virt:pixel_type:backend...",
	 .help = {"pixel_type  - pixel type to be emulated by the backend",
		  "              e.g. G4 or RGB565",
		  "backend     - another backend to be initialized",
		  "              e.g. X11",
		  NULL}
	},
#ifdef HAVE_LIBSDL
	{.name  = "SDL",
	 .init  = sdl_init,
	 .usage = "SDL:[fs]:[8]:[16]:[24]:[32]:[WxH]",
	 .help  = {"fs  - Full Screen mode",
	           "8   - Sets 8bpp",
	           "16  - Sets 16bpp",
	           "24  - Sets 24bpp",
	           "32  - Sets 32bpp",
	           "WxH - Display Size",
	           NULL}
	},
#endif
#ifdef OS_LINUX
	{.name  = "FB",
	 .init  = fb_init,
	 .usage = "fb:[no_shadow]:[new_console]:[/dev/fbX]",
	 .help  = {"no_shadow   - turns off shadow buffer",
	           "new_console - allocate new console",
	           NULL}
	},
#endif
#ifdef HAVE_AALIB
	{.name  = "AA",
	 .init  = aa_init,
	 .usage = "AA",
	 .help  = {NULL}
	},
#endif
	{.name = "help",
	 .init = do_help
	},
	{.name = NULL}
};

static gp_backend *do_help(char *params, const char *caption)
{
	struct backend_init *i;
	unsigned int j;

	(void) params;
	(void) caption;

	for (i = backends; (i+1)->name; i++) {
		fprintf(stderr, "Backend %s\n\n %s\n\n",
		        i->name, i->usage);
		if (i->help[0]) {
			for (j = 0; i->help[j]; j++)
				fprintf(stderr, " %s\n", i->help[j]);
			fprintf(stderr, "\n");
		}
	}

	return NULL;
}

static struct backend_init *get_backend(const char *name)
{
	struct backend_init *i;

	for (i = backends; i->name; i++) {
		if (!strcasecmp(name, i->name))
			return i;
	}

	return NULL;
}

static gp_backend *init_backend(const char *name, char *params,
                                const char *caption)
{
	struct backend_init *init = get_backend(name);
	gp_backend *ret;

	if (!init) {
		GP_WARN("Invalid backend name '%s'", name);
		errno = EINVAL;
		return NULL;
	}

	ret = init->init(params, caption);

	return ret;
}

gp_backend *gp_backend_init(const char *params, const char *caption)
{
	if (params == NULL) {
		do_help(NULL, NULL);
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

	GP_DEBUG(1, "Have backend name '%s' params '%s'", buf, backend_params);

	return init_backend(buf, backend_params, caption);
}
