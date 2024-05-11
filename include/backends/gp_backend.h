// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend.h
 * @brief An abstraction for display, keyboard and mouse.
 *
 * The gp_backend is unified API for managing xserver, framebuffer, waylaynd,
 * drm, e-ink display, etc. along with support for input devices, e.g. mouse,
 * keyboard and keymaps.
 *
 * In contrast to other graphics libraries we do not try to create unified
 * initalization interface that would match specialities for every possible
 * backend. Rather than that we are trying to create API that is the same
 * for all backends, once initalization is done.
 */

#ifndef BACKENDS_GP_BACKEND_H
#define BACKENDS_GP_BACKEND_H

#include <core/gp_types.h>

#include <utils/gp_timer.h>
#include <utils/gp_list.h>
#include <utils/gp_poll.h>

#include <input/gp_ev_queue.h>
#include <input/gp_task.h>

#include <backends/gp_types.h>

enum gp_backend_attrs {
	/* window size */
	GP_BACKEND_SIZE,
	/* window title */
	GP_BACKEND_TITLE,
	/* fullscreen mode */
	GP_BACKEND_FULLSCREEN,
};

/**
 * @brief Cursor types.
 */
enum gp_backend_cursors {
	/** @brief Arrow default cursor type */
	GP_BACKEND_CURSOR_ARROW,
	/** @brief Text edit cursor */
	GP_BACKEND_CURSOR_TEXT_EDIT,
	/** @brief Crosshair */
	GP_BACKEND_CURSOR_CROSSHAIR,
	/** @brief Used typicaly while howering over links */
	GP_BACKEND_CURSOR_HAND,
	/** @brief Last cursor + 1 */
	GP_BACKEND_CURSOR_MAX,

	/** @brief Shows cursor */
	GP_BACKEND_CURSOR_SHOW = 0xc000,
	/** @brief Hides cursor */
	GP_BACKEND_CURSOR_HIDE = 0x8000,
};

/**
 * @brief A backend.
 *
 * This is the main structure that describes a backend API.
 */
struct gp_backend {
	/**
	 * @brief Pointer to pixmap app should draw to.
	 *
	 * In most cases changes are not propagaged into display until
	 * gp_backend_flip() or gp_backend_update_rect() is called.
	 *
	 * This MAY change upon a flip operation.
	 */
	gp_pixmap *pixmap;

	/** @brief Backend name, e.g. "X11" */
	const char *name;

	/**
	 * @brief Updates display.
	 *
	 * If display is buffered, this copies content
	 * of pixmap into display.
	 *
	 * If display is not buffered, this is no-op (set to NULL).
	 */
	void (*flip)(gp_backend *self);

	/**
	 * @brief Updates display rectangle.
	 *
	 * In contrast to flip operation, the pixmap
	 * must not change (this is intended for updating very small areas).
	 *
	 * If display is not buffered, this is no-op (set to NULL).
	 */
	void (*update_rect)(gp_backend *self,
	                    gp_coord x0, gp_coord y0,
	                    gp_coord x1, gp_coord y1);

	/*
	 * Attribute change callback.
	 *
	 * The vals is supposed to be:
	 *
	 * GP_BACKEND_SIZE uint32_t vals[2];
	 * GP_BACKEND_TITLE const char *title
	 * GP_BACKEND_FULLSCREEN int *fs
	 */
	int (*set_attr)(gp_backend *self, enum gp_backend_attrs attr,
			const void *vals);

	/**
	 * @brief Sets cursor shape and/or hides and shows cursor.
	 *
	 * If non NULL backend can enable/disable/change cursor.
	 */
	int (*set_cursor)(gp_backend *self, enum gp_backend_cursors cursor);

	/*
	 * Resize acknowledge callback. This must be called
	 * after you got resize event in order to resize
	 * backend buffers.
	 */
	int (*resize_ack)(gp_backend *self);

	/**
	 * @brief Exits the backend.
	 */
	void (*exit)(gp_backend *self);

	/**
	 * @brief Non-blocking event loop.
	 *
	 * Backends that have a file descriptor does not set this function and
	 * rather insert a file descriptor into the fds array.
	 */
	void (*poll)(gp_backend *self);

	/*
	 * Clipboard handler.
	 */
	int (*clipboard)(gp_backend *self, gp_clipboard *op);

	/**
	 * @brief Blocking event loop. Blocks until events are ready.
	 *
	 * Note that events received by a backend are not necessarily
	 * translated to input events. So input queue may be empty
	 * after Wait has returned.
	 *
	 * Backends that have a file descriptor does not set this function and
	 * rather insert a file descriptor into the fds array.
	 */
	void (*wait)(gp_backend *self);

	/** @brief File descriptors to poll for. */
	gp_poll fds;

	/* @brief Queue to store input events. */
	gp_ev_queue *event_queue;

	/** @brief Priority queue for timers. */
	gp_timer *timers;

	/** @brief Task queue */
	gp_task_queue *task_queue;

	/**
	 * @brief List of input drivers feeding the ev_queue
	 *
	 * A double linked list #gp_backend_input structures.
	 *
	 * The file descriptor has to be registered separatelly to the gp_fds.
	 */
	gp_dlist input_drivers;

	void *clipboard_data;

	/**
	 * @brief A backend DPI if unknown it's set to 0.
	 */
	unsigned int dpi;

	/* Backed private data */
	char priv[];
};

#define GP_BACKEND_PRIV(backend) ((void*)(backend)->priv)

/**
 * @brief Copies whole backend pixmap to a display.
 *
 * Majority of the backends are double buffered, that means that changes done
 * to gp_backend::pixmap are not propagated to the display memory buffer unless
 * they are copied explicitly. This call copies the complete backend pixmap to
 * the display.
 *
 * @param self A backend.
 */
static inline void gp_backend_flip(gp_backend *self)
{
	if (self->flip)
		self->flip(self);
}

/**
 * @brief Copies a rectangle from backend pixmap to a display.
 *
 * Majority of the backends are double buffered, that means that changes done
 * to gp_backend::pixmap are not propagated to the display memory buffer unless
 * they are copied explicitly. This call copies a rectanlge from backend pixmap
 * to the display.
 *
 * @param self A backend.
 * @param x0 First x coordinate of the rectangle.
 * @param y0 First y coordinate of the rectangle.
 * @param x1 Last x coordinate of the rectangle.
 * @param y1 Last y coordinate of the rectangle.
 */
void gp_backend_update_rect_xyxy(gp_backend *self,
                                gp_coord x0, gp_coord y0,
                                gp_coord x1, gp_coord y1);

/**
 * @brief Copies a rectangle from backend pixmap to a display.
 *
 * This is an alias for gp_backend_update_rect_xyxy().
 *
 * @param self A backend.
 * @param x0 First x coordinate of the rectangle.
 * @param y0 First y coordinate of the rectangle.
 * @param x1 Last x coordinate of the rectangle.
 * @param y1 Last y coordinate of the rectangle.
 */
static inline void gp_backend_update_rect(gp_backend *self,
                                          gp_coord x0, gp_coord y0,
                                          gp_coord x1, gp_coord y1)
{
	return gp_backend_update_rect_xyxy(self, x0, y0, x1, y1);
}

/**
 * @brief Copies a rectangle from backend pixmap to a display.
 *
 * This is a version of gp_backend_update_rect_xyxy() with width and height.
 *
 * @param self A backend.
 * @param x Left x coordinate of the rectangle.
 * @param y Top y coordinate of the rectangle.
 * @param w Rectangle width.
 * @param h Rectangle height.
 */
static inline void gp_backend_update_rect_xywh(gp_backend *self,
                                               gp_coord x, gp_coord y,
                                               gp_size w, gp_size h)
{
	gp_backend_update_rect_xyxy(self, x, y, x + w - 1, y + h - 1);
}

static inline void gp_backend_poll_add(gp_backend *self, gp_fd *fd)
{
	gp_poll_add(&self->fds, fd);
}

static inline void gp_backend_poll_rem(gp_backend *self, gp_fd *fd)
{
	gp_poll_rem(&self->fds, fd);
}

static inline gp_fd *gp_backend_poll_rem_by_fd(gp_backend *self, int fd)
{
	return gp_poll_rem_by_fd(&self->fds, fd);
}

static inline int gp_backend_cursor_set(gp_backend *self, enum gp_backend_cursors cursor)
{
	if (self->set_cursor)
		return self->set_cursor(self, cursor);

	return 1;
}

/**
 * @brief Exits the backend.
 *
 * This functions deinitializes the backend. E.g. closes all file descriptors,
 * frees memory, etc. It's important to call this before the application exits
 * since some backends, e.g. framebuffer, cannot be recovered unless we return
 * the underlying facility to the original state.
 *
 * @param self A backend.
 */
void gp_backend_exit(gp_backend *self);

/*
 * Polls backend, the events are filled into event queue.
 */
void gp_backend_poll(gp_backend *self);

/*
 * Poll and GetEvent combined.
 */
gp_event *gp_backend_poll_event(gp_backend *self);

/*
 * Waits for backend events.
 */
void gp_backend_wait(gp_backend *self);

/*
 * Wait and GetEvent combined.
 */
gp_event *gp_backend_wait_event(gp_backend *self);

/*
 * Adds timer to backend.
 *
 * If timer Callback is NULL a timer event is pushed into
 * the backend event queue once timer expires.
 *
 * See input/GP_Timer.h for more information about timers.
 */
void gp_backend_add_timer(gp_backend *self, gp_timer *timer);

/*
 * Removes timer from backend timer queue.
 */
void gp_backend_rem_timer(gp_backend *self, gp_timer *timer);

/*
 * Returns number of timers scheduled in backend.
 */
static inline unsigned int gp_backend_timers_in_queue(gp_backend *self)
{
	return gp_timer_queue_size(self->timers);
}

/*
 * Returns a timeout to a closest timer in ms or -1. Can be passed directly to
 * poll(2).
 */
int gp_backend_timer_timeout(gp_backend *self);

/*
 * Sets backend caption, if supported.
 *
 * When setting caption is not possible/implemented non zero is returned.
 */
static inline int gp_backend_set_caption(gp_backend *backend,
                                         const char *caption)
{
	if (!backend->set_attr)
		return 1;

	return backend->set_attr(backend, GP_BACKEND_TITLE, caption);
}

/*
 * Resize backend, if supported.
 *
 * When resizing is not possible/implemented non zero is returned.
 *
 * When the backend size matches the passed width and height,
 * no action is done.
 *
 * Note that after calling this, the backend->pixmap pointer may change.
 */
int gp_backend_resize(gp_backend *backend, uint32_t w, uint32_t h);

/*
 * Changes fullscreen mode.
 *
 * val 0 - turn off
 *     1 - turn on
 *     2 - toggle
 *     3 - query
 */
static inline int gp_backend_fullscreen(gp_backend *backend, int val)
{
	if (!backend->set_attr)
		return 0;

	return backend->set_attr(backend, GP_BACKEND_FULLSCREEN, &val);
}

/**
 * @brief Resize acknowledge.
 *
 * You must call this right after you application has received resize event.
 * For a multithreaded application all threads that operate on the backend
 * pixmap must be stopped first.
 *
 * After this call returns the backend pixmap has been resized, that means that
 * at least the width, height, and pixels of gp_backend::pixmap are different
 * and the content of the gp_pixmap::pixels is undefined.
 *
 * This call also resizes the windo/display/screen size in the #gp_ev_queue.
 *
 * If the function fails the best action to take is to save application data
 * and exit as the backend may be in undefined state.
 *
 * @param self A backend.
 * @return Zero on success, non-zero otherwise.
 */
int gp_backend_resize_ack(gp_backend *self);

/**
 * @brief Inserts a task into the task queue.
 *
 * Tasks are executed sequentionally and input processing is blocked during
 * task execution.
 *
 * @param self A backend.
 * @param task A task to be inserted into the task queue.
 */
void gp_backend_task_ins(gp_backend *self, gp_task *task);

/**
 * @brief Removes a task from the task queue.
 *
 * @param self A backend.
 * @param task A task to be removed from the task queue.
 */
void gp_backend_task_rem(gp_backend *self, gp_task *task);

/**
 * @brief Sets the backend task_queue and starts task processing.
 *
 * By default backends does not have a task queue populated. Applications that
 * require a task queue have to allocate and initialize the queue then pass it
 * to the backend with this function.
 *
 * @param self A backend.
 * @param task_queue A pointer to initialized task_queue.
 */
void gp_backend_task_queue_set(gp_backend *self, gp_task_queue *task_queue);

/*
 * Event Queue functions.
 */
static inline unsigned int gp_backend_events(gp_backend *self)
{
	return gp_ev_queue_events(self->event_queue);
}

static inline gp_event *gp_backend_get_event(gp_backend *self)
{
	return gp_ev_queue_get(self->event_queue);
}

static inline gp_event *gp_backend_peek_event(gp_backend *self)
{
	return gp_ev_queue_peek(self->event_queue);
}

static inline void gp_backend_put_event_back(gp_backend *self, gp_event *ev)
{
	gp_ev_queue_put_back(self->event_queue, ev);
}

#endif /* BACKENDS_GP_BACKEND_H */
