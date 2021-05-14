// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_debug.h>
#include "core/gp_pixmap.h"
#include <backends/gp_aalib.h>
#include <input/gp_input.h>

#ifndef NO_CONFIG
#include "../config.h"
#endif

#ifdef HAVE_AALIB

#include <aalib.h>
#include <pthread.h>

/*
 * Guards AALib library calls.
 *
 * In case we run under X the underlying xcb connection is not initialized with
 * XInitThreads() and thus not multithread safe.
 */
static pthread_mutex_t aalib_mutex = PTHREAD_MUTEX_INITIALIZER;

struct aalib_priv {
	aa_context *c;
	gp_pixmap pixmap;
};

/* ascii mapped keys */
static const uint16_t keymap[] = {
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 GP_KEY_BACKSPACE,
 GP_KEY_TAB,         0,                 0,                 0,
 GP_KEY_ENTER,       0,                 0,                 0,
 0,                  0,                 GP_KEY_PAUSE,      0,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_ESC,        0,
 0,                  0,                 0,                 GP_KEY_SPACE,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_APOSTROPHE, 0,
 0,                  0,                 0,                 GP_KEY_COMMA,
 GP_KEY_MINUS,       GP_KEY_DOT,        GP_KEY_SLASH,      GP_KEY_0,
 GP_KEY_1,           GP_KEY_2,          GP_KEY_3,          GP_KEY_4,
 GP_KEY_5,           GP_KEY_6,          GP_KEY_7,          GP_KEY_8,
 GP_KEY_9,           0,                 GP_KEY_SEMICOLON,  0,
 GP_KEY_EQUAL,       0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 0,                 0,
 0,                  0,                 GP_KEY_LEFT_BRACE, GP_KEY_BACKSLASH,
 GP_KEY_RIGHT_BRACE, 0,                 0,                 GP_KEY_GRAVE,
 GP_KEY_A,           GP_KEY_B,          GP_KEY_C,          GP_KEY_D,
 GP_KEY_E,           GP_KEY_F,          GP_KEY_G,          GP_KEY_H,
 GP_KEY_I,           GP_KEY_J,          GP_KEY_K,          GP_KEY_L,
 GP_KEY_M,           GP_KEY_N,          GP_KEY_O,          GP_KEY_P,
 GP_KEY_Q,           GP_KEY_R,          GP_KEY_S,          GP_KEY_T,
 GP_KEY_U,           GP_KEY_V,          GP_KEY_W,          GP_KEY_X,
 GP_KEY_Y,           GP_KEY_Z,          0,                 0,
 0,                  0,                 GP_KEY_DELETE,
};

/* special keys mapped from 300 to 305 */
static const uint16_t keymap2[] = {
	GP_KEY_UP, GP_KEY_DOWN,
	GP_KEY_LEFT, GP_KEY_RIGHT,
	GP_KEY_BACKSPACE, GP_KEY_ESC,
};

static void aalib_exit(gp_backend *self)
{
	struct aalib_priv *aa = GP_BACKEND_PRIV(self);

	GP_DEBUG(1, "Closing AALib");
	aa_close(aa->c);
}

static void aalib_flip(gp_backend *self)
{
	struct aalib_priv *aa = GP_BACKEND_PRIV(self);

	GP_DEBUG(4, "Rendering and flipping screen");

	aa_render(aa->c, &aa_defrenderparams,
	          0, 0, aa_scrwidth(aa->c), aa_scrheight(aa->c));

	pthread_mutex_lock(&aalib_mutex);
	aa_flush(aa->c);
	pthread_mutex_unlock(&aalib_mutex);
}

static void aalib_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                              gp_coord x1, gp_coord y1)
{
	struct aalib_priv *aa = GP_BACKEND_PRIV(self);

	GP_DEBUG(4, "Updating rect %ix%i-%ix%i", x0, y0, x1, y1);

	/*
	 * TODO: Map screen coordinates to bitmap coordinates.
	 */
	int w = aa_scrwidth(aa->c);
	int h = aa_scrheight(aa->c);

	aa_render(aa->c, &aa_defrenderparams,
	          0, 0, w, h);

	pthread_mutex_lock(&aalib_mutex);
	aa_flush(aa->c);
	pthread_mutex_unlock(&aalib_mutex);
}

static int aalib_resize_ack(gp_backend *self)
{
	struct aalib_priv *aa = GP_BACKEND_PRIV(self);

	if (!aa_resize(aa->c)) {
		GP_WARN("aa_resize() failed");
		return 1;
	}

	int w = aa_imgwidth(aa->c);
	int h = aa_imgheight(aa->c);

	GP_DEBUG(1, "Reinitializing Pixmap %ix%i", w, h);

	gp_pixmap_init(&aa->pixmap, w, h, GP_PIXEL_G8, aa_image(aa->c));

	return 0;
}

static void parse_event(gp_backend *self, int ev)
{
	unsigned int key;

	if (ev == 0)
		return;

	if (ev == AA_RESIZE) {
		GP_DEBUG(1, "Resize event");
		//TODO: Can we get the new size somehow?
		gp_event_queue_push_resize(&self->event_queue, 0, 0, NULL);
		return;
	}

	if (ev <= (int)GP_ARRAY_SIZE(keymap)) {
		key = keymap[ev - 1];
	} else {
		if (ev >= 300 && ev <= 305) {
			key = keymap2[ev - 300];
		} else {
			GP_DEBUG(1, "Unhandled event %i", ev);
			return;
		}
	}

	/* emulate keyup events */
	gp_event_queue_push_key(&self->event_queue, key, 1, NULL);
	gp_event_queue_push_key(&self->event_queue, key, 0, NULL);
}

static void aalib_poll(gp_backend *self)
{
	struct aalib_priv *aa = GP_BACKEND_PRIV(self);
	int key;

	pthread_mutex_lock(&aalib_mutex);
	key = aa_getevent(aa->c, 0);
	pthread_mutex_unlock(&aalib_mutex);

	parse_event(self, key);
}

static void aalib_wait(gp_backend *self)
{
	/* We cannot wait due to possible lockup, so we poll */
	for (;;) {
		aalib_poll(self);

		if (gp_event_queue_events(&self->event_queue))
			return;

		usleep(10000);
	}
}

gp_backend *gp_aalib_init(void)
{
	gp_backend *backend;
	struct aalib_priv *aa;
	int w, h;

	backend = malloc(sizeof(gp_backend) + sizeof(struct aalib_priv));

	if (backend == NULL)
		return NULL;

	aa = GP_BACKEND_PRIV(backend);

	GP_DEBUG(1, "Initializing aalib");

	aa->c = aa_autoinit(&aa_defparams);

	if (!aa->c) {
		GP_DEBUG(1, "Failed to initialize aalib");
		goto err1;
	}

	GP_DEBUG(1, "AALib driver %s %s %ix%i", aa->c->driver->name,
	         aa->c->driver->shortname,
	         aa->c->params.width, aa->c->params.height);

	if (!aa_autoinitkbd(aa->c, 0)) {
		GP_DEBUG(1, "Failed to initialize aalib keyboard");
		goto err2;
	}

	w = aa_imgwidth(aa->c);
	h = aa_imgheight(aa->c);

	GP_DEBUG(1, "Initializing Pixmap %ix%i", w, h);

	gp_pixmap_init(&aa->pixmap, w, h, GP_PIXEL_G8, aa_image(aa->c));

	backend->pixmap = &aa->pixmap;
	backend->name = "AALib";
	backend->flip = aalib_flip;
	backend->update_rect = aalib_update_rect;
	backend->resize_ack = aalib_resize_ack;
	backend->poll = aalib_poll;
	backend->wait = aalib_wait;
	backend->set_attr = NULL;
	backend->exit = aalib_exit;
	backend->fd = -1;
	backend->timers = NULL;
	gp_event_queue_init(&backend->event_queue, w, h, 0);

	return backend;
err2:
	aa_close(aa->c);
err1:
	free(backend);
	return NULL;
}

#else

gp_backend *gp_aalib_init(void)
{
	GP_FATAL("AALib support not compiled in!");
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_AALIB */
