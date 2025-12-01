// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#include <string.h>
#include <errno.h>

#include <core/gp_debug.h>
#include <core/gp_types.h>

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

static gp_backend *x11_init(char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption)
{
	enum gp_x11_flags flags = 0;

	if (parse_x11_params(params, &pref_w, &pref_h, &flags))
		return NULL;

	return gp_x11_init(NULL, 0, 0, pref_w, pref_h, caption, flags);
}
#endif

#ifdef HAVE_WAYLAND
static gp_backend *wayland_init(char *params,
                                gp_size pref_w, gp_size pref_h,
                                const char *caption)
{
	(void) params;

	return gp_wayland_init(NULL, pref_w, pref_h, caption);
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

static gp_backend *sdl_init(char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption)
{
	gp_size bpp = 0;
	uint8_t flags = GP_SDL_RESIZABLE;

	if (parse_sdl_params(params, &pref_w, &pref_h, &bpp, &flags))
		return NULL;

	return gp_sdl_init(pref_w, pref_h, bpp, flags, caption);
}
#endif

#ifdef OS_LINUX
static int parse_fb_params(char *params, int *flags, const char **fb)
{
	char *param;
	int input = 0;
	int kbd = 0;
	int none = 0;

	if (!params) {
		*flags |= GP_FB_INPUT_LINUX;
		GP_DEBUG(1, "Enabling input=linux");
		return 0;
	}

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

		if (!strcasecmp(param, "input=linux")) {
			input = 1;
			continue;
		}

		if (!strcasecmp(param, "input=kbd")) {
			kbd = 1;
			continue;
		}

		if (!strcasecmp(param, "input=none")) {
			none = 1;
			continue;
		}

		*fb = param;

		if (strncmp(*fb, "/dev/", 5))
			GP_WARN("Console dev set to '%s', are you sure?", *fb);

		GP_DEBUG(1, "Framebuffer console set to '%s'", *fb);

	} while (params);

	if (input + kbd + none > 1) {
		GP_FATAL("Too many input= parameters!");
		return 1;
	}

	if (kbd) {
		*flags |= GP_FB_INPUT_KBD;
		GP_DEBUG(1, "Enabling input=kbd");
		return 0;
	}

	if (!none) {
		*flags |= GP_FB_INPUT_LINUX;
		GP_DEBUG(1, "Enabling input=linux");
		return 0;
	}

	GP_DEBUG(1, "Enabling input=none");
	return 0;
}

static gp_backend *fb_init(char *params,
                           gp_size GP_UNUSED(pref_w), gp_size GP_UNUSED(pref_h),
                           const char GP_UNUSED(*caption))
{
	const char *fb = "/dev/fb0";

	int flags = GP_FB_SHADOW;

	parse_fb_params(params, &flags, &fb);

	return gp_linux_fb_init(fb, flags);
}
#endif

#ifdef HAVE_LINUX_DRM
static int parse_drm_params(char *params, int *flags, const char **dev)
{
	char *param;

	if (!params)
		return 0;

	do {
		param = params;
		params = next_param(params);

		if (!strcasecmp(param, "no_input")) {
			*flags |= GP_LINUX_DRM_NO_INPUT;
			GP_DEBUG(1, "Linux input layer disabled");
			continue;
		}

		if (strncmp(param, "/dev/", 5)) {
			GP_FATAL("Invalid parameter '%s'", *dev);
			return 1;
		}

		*dev = param;
		GP_DEBUG(1, "DRM device set to '%s'", *dev);
	} while (params);

	return 0;
}

static gp_backend *drm_init(char *params,
                            gp_size GP_UNUSED(pref_w), gp_size GP_UNUSED(pref_h),
                            const char GP_UNUSED(*caption))
{
	int flags = 0;
	const char *dev = "/dev/dri/card0";

	if (parse_drm_params(params, &flags, &dev))
		return NULL;

	return gp_linux_drm_init(dev, flags);
}
#endif

#ifdef HAVE_AALIB
static gp_backend *aa_init(char *params,
                           gp_size pref_w, gp_size pref_h,
                           const char *caption)
{
	(void) caption;
	(void) params;
	(void) pref_w;
	(void) pref_h;

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

static gp_backend *xcb_init(char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption)
{
	if (parse_xcb_params(params, &pref_w, &pref_h))
		return NULL;

	return gp_xcb_init(NULL, 0, 0, pref_w, pref_h, caption);
}
#endif

#ifdef OS_LINUX

#include <backends/gp_linux_input.h>

static gp_backend *proxy_init(char *params,
                              gp_size GP_UNUSED(pref_w), gp_size GP_UNUSED(pref_h),
                              const char *caption)
{
	return gp_proxy_init(params, caption);
}

static gp_backend *display_init(char *params,
                                gp_size GP_UNUSED(pref_w), gp_size GP_UNUSED(pref_h),
                                const char GP_UNUSED(*caption))
{
	unsigned int i;

	if (!strcmp(params, "help")) {
		printf("display_models:\n\n");
		for (i = 0; gp_backend_display_models[i].name; i++) {
			printf("\t%s - %s\n",
			       gp_backend_display_models[i].name,
			       gp_backend_display_models[i].desc);
		}

		printf("\n");
		return NULL;
	}

	for (i = 0; gp_backend_display_models[i].name; i++) {
		if (!strcasecmp(gp_backend_display_models[i].name, params)) {
			gp_backend *ret = gp_backend_display_init(i);

			if (gp_linux_input_hotplug_new(ret)) {
				GP_WARN("Failed to initialize Linux input");
				gp_backend_exit(ret);
				return NULL;
			}

			return ret;
		}
	}

	printf("Unknown display model name '%s'\n", params);

	return NULL;
}
#endif

static gp_backend *init_backend(const char *name, char *params,
                                gp_size pref_w, gp_size pref_h,
                                const char *caption);

static gp_backend *virt_init(char *params,
                             gp_size pref_w, gp_size pref_h,
                             const char *caption)
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

	backend = init_backend(backend_name, params, pref_w, pref_h, caption);
	if (!backend)
		return NULL;

	return gp_backend_virt_init(backend, pixel_type, GP_BACKEND_CALL_EXIT);
}

struct backend_init {
	const char *name;
	gp_backend *(*init)(char *params,
	                    gp_size pref_w, gp_size pref_h,
	                    const char *caption);
	const char *usage;
	const char *help[10];
};

static gp_backend *do_help(char *params,
                           gp_size pref_w, gp_size pref_h,
                           const char *caption);

static struct backend_init backends[] = {
#ifdef OS_LINUX
	{.name = "proxy",
	 .init = proxy_init,
	 .usage = "path",
	 .help = {"path - Path to an UNIX socket"}
	},
	{.name = "display",
	 .init = display_init,
	 .usage = "display:display_model",
	 .help = {
		"display_type A display type (pass help for list)",
		NULL
	 }
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
	 .usage = "fb:[no_shadow]:[new_console][input=..]:[/dev/fbX]",
	 .help  = {"no_shadow   - turns off shadow buffer",
	           "new_console - allocate new console",
	           "input=[linux,kbd,none]",
	           "      linux - (default) Linux input for keyboards and pointer devices",
	           "      kbd   - Konsole KBD keyboard",
	           "      none  - No input devices",
	           NULL}
	},
#endif
#ifdef HAVE_LINUX_DRM
	{.name = "DRM",
	 .init = drm_init,
	 .usage = "drm:[/dev/dri/cardX]:[noinput]",
	 .help = {"Linux DRM backend",
		  "no_input - do not use Linux input for keyboard/mouse",
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

static void print_rotation_help(void)
{
	int i;

	fprintf(stderr, "Available rotations:\n--------------------\n\n");

	for (i = 0; gp_symmetry_names[i]; i++)
		fprintf(stderr, "\t - %s\n", gp_symmetry_names[i]);

	fprintf(stderr, "\n");
}

static gp_backend *do_help(char GP_UNUSED(*params),
                           gp_size GP_UNUSED(pref_w), gp_size GP_UNUSED(pref_h),
                           const char GP_UNUSED(*caption))
{
	struct backend_init *i;
	unsigned int j;

	fprintf(stderr, "Usage\n-----\n\n");
	fprintf(stderr, "[rotate=90|180|270|V|H:]backend[:backend_opt:backend_opt...]\n\n");

	print_rotation_help();

	fprintf(stderr, "Backends\n--------\n\n");

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
                                gp_size pref_w, gp_size pref_h,
                                const char *caption)
{
	struct backend_init *init = get_backend(name);
	gp_backend *ret;

	if (!init) {
		GP_WARN("Invalid backend name '%s'", name);
		errno = EINVAL;
		return NULL;
	}

	if (!*params)
		params = NULL;

	ret = init->init(params, pref_w, pref_h, caption);

	return ret;
}

static const char *autodetect_backend(void)
{
	if (getenv("WAYLAND_DISPLAY"))
		return "wayland";

	if (getenv("DISPLAY"))
		return "x11";

	if (getenv("GP_PROXY_PATH"))
		return "proxy";

#ifdef HAVE_LINUX_DRM
	return "drm";
#endif

	return NULL;
}

static char *shift_buf(char *buf)
{
	size_t i;

	for (i = 0; buf[i]; i++) {
		if (buf[i] == ':') {
			buf[i] = '\0';
			return buf + i + 1;
		}
	}

	return buf + i;
}

gp_backend *gp_backend_init(const char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption)
{
	enum gp_symmetry rotation = GP_ROTATE_INVALID;
	gp_backend *ret;

	if (!params)
		params = getenv("GP_BACKEND_INIT");

	if (!params || !params[0]) {
		params = autodetect_backend();

		if (!params) {
			do_help(NULL, 0, 0, NULL);
			return NULL;
		}
	}

	if (!pref_w)
		pref_w = 640;

	if (!pref_h)
		pref_h = 480;

	size_t len = strlen(params);
	char buf[len+1], *backend, *backend_params = NULL;

	strcpy(buf, params);

	backend = buf;

	if (!strncmp(backend, "rotate=", 7)) {
		char *str_rotation = backend + 7;

		backend = shift_buf(backend);

		if (!strcmp(str_rotation, "help")) {
			print_rotation_help();
			return NULL;
		}

		rotation = gp_symmetry_by_name(str_rotation);
		if (rotation == GP_ROTATE_INVALID) {
			GP_FATAL("Invalid rotation '%s'", str_rotation);
			print_rotation_help();
			return NULL;
		}

		GP_DEBUG(1, "Backend rotation set to '%s'", str_rotation);
	}

	backend_params = shift_buf(backend);

	GP_DEBUG(1, "Have backend name '%s' params '%s'", backend, backend_params);

	ret = init_backend(backend, backend_params, pref_w, pref_h, caption);
	//TODO: this is a hack, we need gp_backend_rotate() to rotate the input queue as well
	//      and the flags are lost if backend->pixmap is reallocated as well
	if (ret && rotation != GP_ROTATE_INVALID)
		gp_pixmap_rotate(ret->pixmap, rotation);

	return ret;
}
