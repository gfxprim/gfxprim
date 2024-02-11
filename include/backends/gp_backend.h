// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/*

  The gp_backend is overall structure for API for managing
  connection/mmaped memory/... to xserver window/framebuffer/... .

  In contrast to other graphics libraries we do not try to create unified
  initalization interface that would match specialities for every possible
  backend. Rather than that we are trying to create API that is the same
  for all backends, once initalization is done.

  So once you initalize, for example, framebuffer driver, the gp_backend
  structure is returned, which then could be used with generic code for
  backend drawing.

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

enum gp_backend_cursors {
	/* Arrow default cursor type */
	GP_BACKEND_CURSOR_ARROW,
	/* Text edit cursor */
	GP_BACKEND_CURSOR_TEXT_EDIT,
	/* Crosshair */
	GP_BACKEND_CURSOR_CROSSHAIR,
	/* Used typicaly while howering over links */
	GP_BACKEND_CURSOR_HAND,
	/* Last cursor + 1 */
	GP_BACKEND_CURSOR_MAX,

	/* Shows cursor */
	GP_BACKEND_CURSOR_SHOW = 0xc000,
	/* Hides cursor */
	GP_BACKEND_CURSOR_HIDE = 0x8000,
};

struct gp_backend {
	/*
	 * Pointer to pixmap app should draw to.
	 *
	 * This MAY change upon a flip operation.
	 */
	gp_pixmap *pixmap;

	/* Backend name */
	const char *name;

	/*
	 * If display is buffered, this copies content
	 * of pixmap into display.
	 *
	 * If display is not buffered, this is no-op (set to NULL).
	 */
	void (*flip)(gp_backend *self);

	/*
	 * Updates display rectangle.
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
	 * If non NULL backend can enable/disable/change cursor.
	 */
	int (*set_cursor)(gp_backend *self, enum gp_backend_cursors cursor);

	/*
	 * Resize acknowledge callback. This must be called
	 * after you got resize event in order to resize
	 * backend buffers.
	 */
	int (*resize_ack)(gp_backend *self);

	/*
	 * Exits the backend.
	 */
	void (*exit)(gp_backend *self);

	/*
	 * Non-blocking event loop.
	 *
	 * Backends that have a file descriptor does not set this function and
	 * rather insert a file descriptor into the fds array.
	 */
	void (*poll)(gp_backend *self);

	/*
	 * Clipboard handler.
	 */
	int (*clipboard)(gp_backend *self, gp_clipboard *op);

	/*
	 * Blocking event loop. Blocks until events are ready.
	 *
	 * Note that events received by a backend are not necessarily
	 * translated to input events. So input queue may be empty
	 * after Wait has returned.
	 *
	 * Backends that have a file descriptor does not set this function and
	 * rather insert a file descriptor into the fds array.
	 */
	void (*wait)(gp_backend *self);

	/* File descriptors to poll for */
	gp_poll fds;

	/*
	 * Queue to store input events.
	 */
	gp_ev_queue *event_queue;

	/* Priority queue for timers. */
	gp_timer *timers;

	/* Task queue */
	gp_task_queue *task_queue;

	/**
	 * List of input drivers feeding the ev_queue
	 *
	 * The file descriptor has to be registered separatelly to the gp_fds.
	 */
	gp_dlist input_drivers;

	void *clipboard_data;

	/**
	 * DPI if unknown it's set to 0.
	 */
	unsigned int dpi;

	/* Backed private data */
	char priv[];
};

#define GP_BACKEND_PRIV(backend) ((void*)(backend)->priv)

static inline void gp_backend_flip(gp_backend *self)
{
	if (self->flip)
		self->flip(self);
}

void gp_backend_update_rect_xyxy(gp_backend *self,
                                gp_coord x0, gp_coord y0,
                                gp_coord x1, gp_coord y1);

static inline void gp_backend_update_rect(gp_backend *self,
                                          gp_coord x0, gp_coord y0,
                                          gp_coord x1, gp_coord y1)
{
	return gp_backend_update_rect_xyxy(self, x0, y0, x1, y1);
}

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

/*
 * Resize acknowledge. You must call this right after you application has
 * received resize event.
 *
 * This will resize backend buffers. After this call returns the backend width
 * height and pixmap pointer are most likely different.
 *
 * This function returns zero on succes. Non zero on failure. If it fails the
 * best action to take is to save application data and exit (as the backend
 * may be in undefined state).
 */
int gp_backend_resize_ack(gp_backend *self);

/**
 * @brief Inserts a task into the task queue.
 *
 * Tasks are executed sequentionally and input processing is blocked during
 * task execution.
 *
 * @self A backend.
 * @task A task to be inserted into the task queue.
 */
void gp_backend_task_ins(gp_backend *self, gp_task *task);

/**
 * @brief Removes a task from the task queue.
 *
 * @self A backend.
 * @task A task to be removed from the task queue.
 */
void gp_backend_task_rem(gp_backend *self, gp_task *task);

/**
 * @brief Sets the backend task_queue and starts task processing.
 *
 * By default backends does not have a task queue populated. Applications that
 * require a task queue have to allocate and initialize the queue then pass it
 * to the backend with this function.
 *
 * @self A backend.
 * @task_queue A pointer to initialized task_queue.
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
