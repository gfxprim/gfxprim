// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 * Copyright (C) 2023 Jiri Dluhos <jiri.bluebear.dluhos@gmail.com>
 */

#include "../../config.h"

#include <core/gp_debug.h>
#include <core/gp_pixmap.h>
#include <utils/gp_utf.h>
#include <backends/gp_backends.h>

#ifdef HAVE_WAYLAND

#include <wayland-util.h>
#include <wayland-version.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define UN(x) x __attribute__((unused))

/* Shared memory support code */
static void
randname(char *buf)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int
create_shm_file(void)
{
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		randname(name + sizeof(name) - 7);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}

static int
allocate_shm_file(size_t size)
{
	int fd = create_shm_file();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

struct client_state {
	/* display */
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_shm *shm;
	struct wl_compositor *compositor;
	struct xdg_wm_base *wm_base;

	/* input */
	struct wl_seat *seat;
	struct wl_keyboard *keyboard;
	struct wl_pointer *pointer;

	/* keymap */
	struct xkb_context *keymap_context;
	struct xkb_state *keymap_state;
	struct xkb_keymap *keymap;

	/* window */
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	/* window decoration */
	struct zxdg_decoration_manager_v1 *decoration_manager;
	struct zxdg_toplevel_decoration_v1 *decoration;

	/* gfxprim */
	uint32_t w, h;
	gp_pixel pixel_type;
	gp_ev_queue ev_queue;

	gp_backend *backend;
};

static struct client_state state = {};

struct buffered_frame {
	int width;
	int height;
	uint32_t* data;
	struct wl_buffer* buffer;
};

static struct buffered_frame frame = {};

static int buffered_frame_init(struct client_state* state, struct buffered_frame *frame)
{
	const int stride = state->w * 4;
	const int size = stride * state->h;

	int fd = allocate_shm_file(size);
	if (fd == -1) {
		return 0;
	}

	uint32_t *data = mmap(NULL, size,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		close(fd);
		return 0;
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd, size);

	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
			state->w, state->h, stride, WL_SHM_FORMAT_XRGB8888);

	wl_shm_pool_destroy(pool);
	close(fd);

	frame->width = state->w;
	frame->height = state->h;
	frame->data = data;
	frame->buffer = buffer;
	return 1;
}

static void
xdg_surface_configure(void UN(*data),
                      struct xdg_surface *xdg_surface, uint32_t serial)
{

	xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial)
{
	(void) data;

	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping,
};

static void shm_format(void *data, struct wl_shm *shm, uint32_t format)
{
	struct client_state *state = data;

	(void) shm;

	if (state->pixel_type != GP_PIXEL_UNKNOWN)
		return;

	switch (format) {
	case WL_SHM_FORMAT_XRGB8888:
		state->pixel_type = GP_PIXEL_xRGB8888;
	break;
	case WL_SHM_FORMAT_RGB888:
		state->pixel_type = GP_PIXEL_RGB888;
	break;
	}

	if (state->pixel_type == GP_PIXEL_UNKNOWN)
		return;

	GP_DEBUG(1, "SHM pixel format %s (%"PRIx32")",
	         gp_pixel_type_name(state->pixel_type), format);
}

static struct wl_shm_listener shm_listener = {
	.format = shm_format};

static void
pointer_handle_enter(void UN(*data), struct wl_pointer UN(*pointer),
                     uint32_t UN(serial), struct wl_surface UN(*surface),
		     wl_fixed_t UN(sx), wl_fixed_t UN(sy))
{
}

static void
pointer_handle_leave(void UN(*data), struct wl_pointer UN(*pointer),
                     uint32_t UN(serial), struct wl_surface UN(*surface))
{
}

static void
pointer_handle_motion(void UN(*data), struct wl_pointer UN(*pointer),
		      uint32_t UN(time), wl_fixed_t sx, wl_fixed_t sy)
{
	struct client_state *state = data;
	gp_backend *backend = state->backend;

	gp_ev_queue_push_rel_to(backend->event_queue, sx/256, sy/256, 0);
}

static void
pointer_handle_button(void *data, struct wl_pointer UN(*pointer),
                      uint32_t UN(serial), uint32_t UN(time), uint32_t button,
                      uint32_t button_state)
{
	struct client_state *state = data;
	gp_backend *backend = state->backend;

	gp_ev_queue_push_key(backend->event_queue, button, button_state, 0);
}

static void
pointer_handle_axis(void UN(*data), struct wl_pointer UN(*pointer),
		    uint32_t UN(time), uint32_t axis,  wl_fixed_t value)
{
	struct client_state *state = data;
	gp_backend *backend = state->backend;

	switch (axis) {
	case WL_POINTER_AXIS_SOURCE_WHEEL:
		gp_ev_queue_push(backend->event_queue, GP_EV_REL, GP_EV_REL_WHEEL, value/(15 * 256), 0);
	break;
	}
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void
keyboard_handle_keymap(void *data, struct wl_keyboard UN(*keyboard),
                       uint32_t format, int fd, uint32_t size)
{
	struct client_state *state = data;

	GP_DEBUG(0, "keymap change");

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		GP_WARN("Unknown keymap format!");
		goto err0;

	}

	char *keymap_str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (keymap_str == MAP_FAILED) {
		GP_WARN("Failed to map keymap");
		goto err0;
	}

	xkb_keymap_unref(state->keymap);
	xkb_context_unref(state->keymap_context);
	xkb_state_unref(state->keymap_state);

	state->keymap_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	state->keymap = xkb_keymap_new_from_string(state->keymap_context,
	                                           keymap_str,
	                                           XKB_KEYMAP_FORMAT_TEXT_V1,
	                                           XKB_KEYMAP_COMPILE_NO_FLAGS);
	state->keymap_state = xkb_state_new(state->keymap);

	munmap(keymap_str, size);

	GP_DEBUG(1, "Keymap created");
err0:
	close(fd);
}

static void
keyboard_handle_enter(void UN(*data), struct wl_keyboard UN(*keyboard),
                      uint32_t UN(serial), struct wl_surface UN(*surface),
                      struct wl_array UN(*keys))
{
}

static void
keyboard_handle_leave(void UN(*data), struct wl_keyboard UN(*keyboard),
                      uint32_t UN(serial), struct wl_surface UN(*surface))
{
}

static void
keyboard_handle_key(void *data, struct wl_keyboard UN(*keyboard),
                    uint32_t UN(serial), uint32_t UN(time), uint32_t key,
                    uint32_t key_state)
{
	struct client_state *state = data;
	gp_backend *backend = state->backend;
	int ret;
	char buf[128];

	gp_ev_queue_push_key(backend->event_queue, key, key_state, 0);

	ret = xkb_state_key_get_utf8(state->keymap_state, key+8, buf, sizeof(buf));
	if (ret) {
		uint32_t unicode;
		const char *str = buf;

		while ((unicode = gp_utf8_next(&str)))
			gp_ev_queue_push_utf(backend->event_queue, unicode, 0);
	}
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard UN(*keyboard),
                          uint32_t UN(serial), uint32_t mods_depressed,
                          uint32_t mods_latched, uint32_t mods_locked,
                          uint32_t group)
{
	struct client_state *state = data;

	xkb_state_update_mask(state->keymap_state,
	                      mods_depressed, mods_latched, mods_locked,
	                      0, 0, group);
}

static const struct wl_keyboard_listener keyboard_listener = {
	.keymap = keyboard_handle_keymap,
	.enter = keyboard_handle_enter,
	.leave = keyboard_handle_leave,
	.key = keyboard_handle_key,
	.modifiers = keyboard_handle_modifiers,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat,
                                     enum wl_seat_capability caps)
{
	struct client_state *state = data;

        if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !state->keyboard) {
                state->keyboard = wl_seat_get_keyboard(seat);
                wl_keyboard_add_listener(state->keyboard, &keyboard_listener, state);
        } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && state->keyboard) {
                wl_keyboard_destroy(state->keyboard);
                state->keyboard = NULL;
        }

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !state->pointer) {
		state->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(state->pointer, &pointer_listener, state);
	} else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && state->pointer) {
		wl_pointer_destroy(state->pointer);
		state->pointer = NULL;
	}
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
};

static void configure_decorations(void UN(*data),
                                  struct zxdg_toplevel_decoration_v1 UN(*zxdg_toplevel_decoration_v1),
                                  uint32_t mode)
{
	if (mode != ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
		GP_WARN("Failed to turn on server side decorations!");
}

static struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
	configure_decorations,
};

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface, uint32_t version)
{
	struct client_state *state = data;

	(void) version;

	GP_DEBUG(5, "Wayland interface '%s'", interface);

	if (!strcmp(interface, "wl_shm")) {
		state->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		wl_shm_add_listener(state->shm, &shm_listener, state);
		return;
	}

	if (!strcmp(interface, "wl_compositor")) {
		state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
		return;
	}

	if (!strcmp(interface, "xdg_wm_base")) {
		state->wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(state->wm_base,
				&xdg_wm_base_listener, state);
		return;
	}

	if (!strcmp(interface, "wl_seat")) {
		state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(state->seat, &seat_listener, state);
		return;
	}

	if (!strcmp(interface, "zxdg_decoration_manager_v1")) {
		if (version != 1) {
			GP_WARN("Unexpected decoration manager version %"PRIu32, version);
			return;
		}
		state->decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
		return;
	}
}

static void
registry_global_remove(void UN(*data),
		struct wl_registry UN(*registry), uint32_t UN(name))
{
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

static void display_disconnect(struct client_state *state)
{
	if (state->shm)
		wl_shm_destroy(state->shm);

	if (state->wm_base)
		xdg_wm_base_destroy(state->wm_base);

	if (state->compositor)
		wl_compositor_destroy(state->compositor);

	wl_registry_destroy(state->registry);
	wl_display_flush(state->display);
	wl_display_disconnect(state->display);
}

static int display_connect(const char *disp_name, struct client_state *state)
{
	state->pixel_type = GP_PIXEL_UNKNOWN;

	state->display = wl_display_connect(disp_name);
	if (!state->display) {
		GP_FATAL("wl_display_connect() failed, is a Wayland compositor running?");
		return 1;
	}

	state->registry = wl_display_get_registry(state->display);

	wl_registry_add_listener(state->registry, &wl_registry_listener, state);

	wl_display_roundtrip(state->display);

	if (!state->shm) {
		GP_FATAL("wl_shm not supported!");
		display_disconnect(state);
		return 1;
	}

	/* we added shm listener in the first roundtrip trigger it with second call */
	wl_display_roundtrip(state->display);

	if (state->pixel_type == GP_PIXEL_UNKNOWN) {
		GP_FATAL("Failed to match a pixel type");
		display_disconnect(state);
		return 1;
	}

	return 0;
}

static void window_destroy(struct client_state *state)
{
	if (state->xdg_toplevel)
		xdg_toplevel_destroy(state->xdg_toplevel);

	if (state->xdg_surface)
		xdg_surface_destroy(state->xdg_surface);

	wl_surface_destroy(state->surface);
}

static void toplevel_configure(void *data, struct xdg_toplevel UN(*toplevel),
                               int32_t w, int32_t h, struct wl_array UN(*states))
{
	struct client_state *st = data;

	gp_ev_queue_push_resize(st->backend->event_queue, w, h, 0);
}

static void toplevel_close(void *data, struct xdg_toplevel UN(*toplevel))
{
	struct client_state *st = data;

	gp_ev_queue_push(st->backend->event_queue, GP_EV_SYS, GP_EV_SYS_QUIT, 0, 0);
}

static const struct xdg_toplevel_listener toplevel_listener = {
	.configure = toplevel_configure,
	.close = toplevel_close,
};

static int window_create(struct client_state *state, unsigned int w, unsigned int h, const char *caption)
{
	state->w = w;
	state->h = h;

	state->surface = wl_compositor_create_surface(state->compositor);
	state->xdg_surface = xdg_wm_base_get_xdg_surface(state->wm_base, state->surface);

	if (!state->xdg_surface) {
		GP_FATAL("Failed to get xdg_surface");
		window_destroy(state);
		return 1;
	}

	xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);

	state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
	if (!state->xdg_toplevel) {
		GP_FATAL("Failed to get xdg_toplevel");
		window_destroy(state);
		return 1;
	}

	xdg_toplevel_add_listener(state->xdg_toplevel, &toplevel_listener, state);
	xdg_toplevel_set_app_id(state->xdg_toplevel, caption);
	xdg_toplevel_set_title(state->xdg_toplevel, caption);

	if (state->decoration_manager) {
		state->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(state->decoration_manager, state->xdg_toplevel);
		zxdg_toplevel_decoration_v1_add_listener(state->decoration, &decoration_listener, state);
		zxdg_toplevel_decoration_v1_set_mode(state->decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}

	buffered_frame_init(state, &frame);

	wl_surface_attach(state->surface, frame.buffer, state->w, state->h);
	wl_surface_commit(state->surface);

	return 0;
}

static void wayland_flip(gp_backend *self)
{
	(void) self;

	//TODO: we need two buffers and swap the backend->pixmap on attach
	//      so that we avoid drawing into a memory that is currently
	//      attached to surface
	wl_surface_attach(state.surface, frame.buffer, state.w, state.h);
	wl_surface_damage(state.surface, 0, 0, state.w, state.h);
	wl_surface_commit(state.surface);
	wl_display_flush(state.display);
}


static void wayland_update_rect(gp_backend* self, gp_coord x, gp_coord y, gp_coord w, gp_coord h)
{
	(void) self;

	wl_surface_attach(state.surface, frame.buffer, state.w, state.h);
	wl_surface_damage(state.surface, x, y, w, h);
	wl_surface_commit(state.surface);
	wl_display_flush(state.display);
}

static int wayland_set_attr(gp_backend* self, enum gp_backend_attrs attrs, const void* values)
{
	(void) self;
	(void) attrs;
	(void) values;

	return 0;
}

static int wayland_process_fd(gp_fd *self)
{
	(void) self;

	wl_display_dispatch(state.display);

	return 0;
}

static int wayland_resize_ack(gp_backend* self)
{
	(void) self;

	return 0;
}

static void wayland_exit(gp_backend* self)
{
	(void) self;

	/* Free keymap */
	xkb_keymap_unref(state.keymap);
	xkb_context_unref(state.keymap_context);
	xkb_state_unref(state.keymap_state);

	window_destroy(&state);
	display_disconnect(&state);
}

static struct gp_backend backend = {
	.name = "Wayland",
	.flip = wayland_flip,
	.update_rect = wayland_update_rect,
	.set_attr = wayland_set_attr,
	.resize_ack = wayland_resize_ack,
	.exit = wayland_exit,
};

gp_backend *gp_wayland_init(const char *display,
                            gp_size w, gp_size h, const char *caption)
{
	if (display_connect(display, &state))
		return NULL;

	int fd = wl_display_get_fd(state.display);

	if (gp_fds_add(&backend.fds, fd, POLLIN, wayland_process_fd, &backend)) {
		display_disconnect(&state);
		return NULL;
	}

	if (window_create(&state, w, h, caption)) {
		gp_fds_rem(&backend.fds, fd);
		display_disconnect(&state);
		return NULL;
	}

	state.backend = &backend;

	backend.pixmap = gp_pixmap_alloc(w, h, state.pixel_type);

	backend.pixmap->pixels = (void*)frame.data;

	backend.event_queue = &state.ev_queue;

	gp_ev_queue_init(backend.event_queue, w, h, 0, 0);

	return &backend;
}

#else

gp_backend *gp_wayland_init(const char *display,
                            gp_size w, gp_size h, const char *caption)
{
	(void) display;
	(void) w;
	(void) h;
	(void) caption;

	GP_FATAL("Wayland support not compiled in!");

	return NULL;
}

#endif /* HAVE_WAYLAND */
