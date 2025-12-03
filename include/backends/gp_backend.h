// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend.h
 * @brief An abstraction for display, keyboard and mouse.
 *
 * The gp_backend is unified API for managing xserver, framebuffer, waylaynd,
 * drm, e-ink display, etc. along with support for input devices, e.g. mouse,
 * keyboard and keymaps. It also implements timers and tasks so that work can
 * be scheduled.
 *
 * In contrast to other graphics libraries we do not try to create unified
 * initalization interface that would match specialities for every possible
 * backend. Rather than that we are trying to create API that is the same
 * for all backends, once initalization is done. This is implemented by
 * optional backend attributes that may or may not be supported by a backend.
 */

#ifndef BACKENDS_GP_BACKEND_H
#define BACKENDS_GP_BACKEND_H

#include <core/gp_types.h>
#include <core/gp_pixmap.h>

#include <utils/gp_timer.h>
#include <utils/gp_list.h>
#include <utils/gp_poll.h>

#include <input/gp_ev_queue.h>
#include <input/gp_task.h>

#include <backends/gp_types.h>

/**
 * @brief Backend attributes.
 */
enum gp_backend_attr {
	/**
	 * @brief Window change request.
	 *
	 * The attribute value is an array with two integers.
	 */
	GP_BACKEND_ATTR_SIZE,
	/**
	 * @brief Window title change request.
	 *
	 * The attribute value is an UTF8 string.
	 */
	GP_BACKEND_ATTR_TITLE,
	/**
	 * @brief Fullscreen mode change request.
	 *
	 * The attribute value is enum gp_backend_fullscreen_req.
	 */
	GP_BACKEND_ATTR_FULLSCREEN,
	/**
	 * @brief Cursor modifications.
	 *
	 * The attribute value is enum gp_backend_cursor_req.
	 */
	GP_BACKEND_ATTR_CURSOR,
	/**
	 * @brief Backlight support.
	 *
	 * Only a few backends need to support backlight, at the moment it's
	 * mostly Linux DRM.
	 */
	GP_BACKEND_ATTR_BACKLIGHT,
};

/**
 * @brief A return value from the backend set_attr callback.
 *
 * All error states are negative, for fucntions that sets attribute success is
 * 0 and for functions that query an attribute the return is either >= 0 for
 * success, or negative for errors.
 */
enum gp_backend_ret {
	/** @brief Atrribute set succesfully. */
	GP_BACKEND_OK = 0,
	/** @brief Attribute is disabled. */
	GP_BACKEND_OFF = 0,
	/** @brief Attribute is enabled. */
	GP_BACKEND_ON = 1,
	/**
	 * @brief Unsupported attribute.
	 *
	 * Not all backends support all attributes.
	 */
	GP_BACKEND_NOTSUPP = -1,
	/**
	 * @brief Invalid value.
	 *
	 * E.g. an attempt to set windows size to 0x0 pixels or resize window
	 * in a fullscreen mode.
	 */
	GP_BACKEND_EINVAL = -2,
	/** @brief Connection error. */
	GP_BACKEND_CONNERR = -3,
};

/**
 * @brief Fullscreen request type.
 */
enum gp_backend_fullscreen_req {
	/** @brief Request fullscreen to be turned off. */
	GP_BACKEND_FULLSCREEN_OFF = 0,
	/** @brief Request fullscreen to be turned on. */
	GP_BACKEND_FULLSCREEN_ON = 1,
	/** @brief Toggle fullscreen state. */
	GP_BACKEND_FULLSCREEN_TOGGLE = 2,
	/**
	 * @brief Query fullscreen state.
	 *
	 * Returns the fullscreen state one of #GP_BACKEND_ON, #GP_BACKEND_OFF
	 * or an error e.g. #GP_BACKEND_NOTSUPP if fullscreen is not supported.
	 */
	GP_BACKEND_FULLSCREEN_QUERY = 3,
};

/**
 * @brief Cursor types.
 */
enum gp_backend_cursor_req {
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
 * @brief Backlight operations.
 */
enum gp_backend_backlight_req {
	/**
	 * @brief Increases the backlight brightness.
	 *
	 * Returns current backlight intensity in percents.
	 */
	GP_BACKEND_BACKLIGHT_INC,
	/**
	 * @brief Decreases the backlight brightness.
	 *
	 * Returns current backlight intensity in percents.
	 */
	GP_BACKEND_BACKLIGHT_DEC,
	/**
	 * @brief Returns current backlight intensity in percents.
	 */
	GP_BACKEND_BACKLIGHT_GET,
};

/**
 * @brief A backend.
 *
 * This is the main structure that describes a backend API.
 */
struct gp_backend {
	/**
	 * @brief Pointer to a pixmap an app should draw to.
	 *
	 * In most cases changes are not updated on a display until
	 * gp_backend_flip(), gp_backend_update() or gp_backend_update_rect()
	 * is called.
	 *
	 * Generally there are three different possible ways how data are
	 * updated on a display.
	 *
	 * - Directly mapped display buffer (e.g. old Linux framebuffer) the
	 *   pixmap data points directly to a DMA buffer that is periodically
	 *   send to the display. In this case the gp_backend::flip,
	 *   gp_backend::update, and gp_backend::update_rect pointers are set
	 *   to NULL.
	 *
	 * - Buffers are atomically swapped on update (e.g. Linux DRM). The
	 *   gp_backend::update_rect is NULL, the gp_backend_update_rect()
	 *   calls gp_backend_update() and gp_backend::update does copy the
	 *   old buffer to the new buffer before buffers are swapped.
	 *
	 * - Buffer is send over a bus to a display (e.g. e-ink or LCD SPI
	 *   display). In this case the gp_backend::flip pointer is NULL and
	 *   gp_backend_flip() calls gp_backend::update.
	 *
	 * @warning The pixmap or gp_pixmap::pixels pointers MAY change when data are
	 *          updated on a display.
	 */
	gp_pixmap *pixmap;

	/** @brief Backend name, e.g. "X11" */
	const char *name;

	/**
	 * @brief Flips backend and display buffers.
	 *
	 * This callback swaps buffers between gp_backend::pixmap and
	 * currently displayed buffer. Some backends, e.g. SPI displays, does
	 * not support this operation, the pointer is NULL and
	 * gp_backend_flip() falls back to gp_backend_update().
	 */
	void (*flip)(gp_backend *self);

	/**
	 * @brief Updates whole display buffer.
	 *
	 * Updates a display to show an image from gp_backend::pixmap. The
	 * gp_backend::pixmap data (not necesarilly the gp_pixmap or
	 * gp_pixmap::pixels pointers) will keep the image data. For some
	 * backends, e.g. Linux DRM this means we are doing a memcpy() from the
	 * buffer we are going to display to a buffer that the application will
	 * write into.
	 *
	 * This pointer may be NULL if gp_pixmap::pixels points directly to a
	 * display DMA buffer and in this case the gp_backend_update() is
	 * no-op.
	 */
	void (*update)(gp_backend *self);

	/**
	 * @brief Updates display rectangle.
	 *
	 * Updates a (part) of the gp_backend::pixmap buffer on the display.
	 *
	 * Some backends does not support partial update (e.g. Linux DRM) and
	 * always do a full display update. Other backends needs to round the
	 * coordinates to a byte boundary (e.g. E-Ink display). That means that
	 * the whole buffer must contain valid data even if we do partial
	 * update.
	 */
	void (*update_rect)(gp_backend *self,
	                    gp_coord x0, gp_coord y0,
	                    gp_coord x1, gp_coord y1);

	/**
	 * @brief Attribute change callback.
	 *
	 * This callback is not supposed to be used directly. Use the nice
	 * helpers such as gp_backend_resize() or gp_backend_fullscreen().
	 *
	 * @param self A backend.
	 * @param attr Which attribute to set.
	 * @param vals An attribute value, the type depends on attribute.
	 *
	 * @return Returns if attribute was succesfully set or a value in
	 *         case of a attribute query.
	 */
	enum gp_backend_ret (*set_attr)(gp_backend *self,
	                                enum gp_backend_attr attr,
	                                const void *vals);

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
	void (*wait)(gp_backend *self, int timeout_ms);

	/** @brief File descriptors to poll for. */
	gp_poll fds;

	/** @brief Queue to store input events. */
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

	/**
	 * @brief Clipboard data.
	 *
	 * Pointer to be used by the backend clipboard implementation.
	 */
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
 * @brief Returns backend width.
 *
 * @param self A backend.
 * @return A backend pixmap width.
 */
static inline gp_size gp_backend_w(gp_backend *self)
{
	return gp_pixmap_w(self->pixmap);
}

/**
 * @brief Returns backend height.
 *
 * @param self A backend.
 * @return A backend pixmap height.
 */
static inline gp_size gp_backend_h(gp_backend *self)
{
	return gp_pixmap_h(self->pixmap);
}

/**
 * @brief Returns backend pixel type.
 *
 * @param self A backend.
 * @return A backend pixel type.
 */
static inline gp_pixel_type gp_backend_pixel_type(gp_backend *self)
{
	return self->pixmap->pixel_type;
}

/**
 * @brief Copies a rectangle from backend pixmap to a display.
 *
 * @important This function should be used when application draws small partial
 *            updates of the display, e.g. appending letters into a textbox.
 *
 * Updates a rectangle from a gp_pixmap::pixels buffer on the display.
 *
 * @warning Some backends does not support partial update (e.g. Linux DRM)
 *          and always do a full display update. Other backends needs to
 *          round the coordinates to a byte boundary (e.g. E-Ink display).
 *          That means that the whole gp_pixmap::pixels buffer must contain
 *          a valid data even if we do partial update.
 *
 * This call is no-op for backends that write directly into the display DMA
 * buffer.
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

/**
 * @brief Copies data from backend buffer to display.
 *
 * This function copies the data from the backend pixmap to the displayed
 * buffer. Unlike the gp_backend_flip() the pixmap data are kept after the
 * operation. For some backends, e.g. Linux DRM this means that this function
 * does memcpy() of the buffer we are about to display. For SPI display
 * backends this sends the whole gp_backend::pixmap pixels buffer to the
 * display over SPI.
 *
 * This call is no-op for backends that write directly into the display DMA
 * buffer.
 *
 * @param self A backend.
 */
static inline void gp_backend_update(gp_backend *self)
{
	if (self->update)
		self->update(self);
}

/**
 * @brief Flips backend and display buffers.
 *
 * @important This call should be used when application generates each frame
 *            from a scratch and send them to a display e.g. video player.
 *
 * In most cases this function swaps buffers between the gp_backend::pixmap
 * and display. If flipping buffers is not supported, e.g. for displays
 * connected over SPI the gp_backend::flip pointer is NULL and the operation
 * falls back to gp_backend_update().
 *
 * @warning After this call the content of the gp_backend::pixmap or the
 *          location of its memory may change.
 *
 * This call is no-op for backends that write directly into the display DMA
 * buffer.
 *
 * @param self A backend.
 */
static inline void gp_backend_flip(gp_backend *self)
{
	if (self->flip)
		self->flip(self);
	else
		gp_backend_update(self);
}

/**
 * @brief Add a file descriptor to the backend poll loop.
 *
 * @param self A backend.
 * @param fd A file descriptor with a callback.
 */
static inline void gp_backend_poll_add(gp_backend *self, gp_fd *fd)
{
	gp_poll_add(&self->fds, fd);
}

/**
 * @brief Removes a file descriptor to the backend poll loop.
 *
 * @param self A backend.
 * @param fd A file descriptor with a callback.
 */
static inline void gp_backend_poll_rem(gp_backend *self, gp_fd *fd)
{
	gp_poll_rem(&self->fds, fd);
}

/**
 * @brief Removes a file descriptor to the backend poll loop.
 *
 * @param self A backend.
 * @param fd A file descriptor used to look up the #gp_fd structure.
 */
static inline gp_fd *gp_backend_poll_rem_by_fd(gp_backend *self, int fd)
{
	return gp_poll_rem_by_fd(&self->fds, fd);
}

/**
 * @brief Sets backend cursor type.
 *
 * If supported changes backend cursor to the requested type, disables or
 * enables a cursor.
 *
 * @param self A backend.
 * @param cursor A cursor type.
 *
 * @return A #GP_BACKEND_OK if cursor was modified, #GP_BACKEND_NOTSUPP if
 *         backend cursor cannot be set.
 */
static inline enum gp_backend_ret gp_backend_cursor_set(gp_backend *self,
                                                        enum gp_backend_cursor_req cursor)
{
	if (self->set_attr)
		return self->set_attr(self, GP_BACKEND_ATTR_CURSOR, &cursor);

	return GP_BACKEND_NOTSUPP;
}

/**
 * @brief Increases the backlight intensity.
 *
 * There are a few backends that can modify the backlight intensity, e.g. Linux
 * DRM. For these backends the backlight can be increased in several steps.
 *
 * @param self A backend.
 * @return A new backlight intensity in percents or GP_BACKEND_NOTSUPP if
 *         backlight is not supported by the backend.
 */
static inline int gp_backend_backlight_inc(gp_backend *self)
{
	enum gp_backend_backlight_req req = GP_BACKEND_BACKLIGHT_INC;

	if (self->set_attr)
		return self->set_attr(self, GP_BACKEND_ATTR_BACKLIGHT, &req);

	return GP_BACKEND_NOTSUPP;
}

/**
 * @brief Decreases the backlight intensity.
 *
 * There are a few backends that can modify the backlight intensity, e.g. Linux
 * DRM. For these backends the backlight can be decreased in several steps.
 *
 * @param self A backend.
 * @return A new backlight intensity in percents or GP_BACKEND_NOTSUPP if
 *         backlight is not supported by the backend.
 */
static inline int gp_backend_backlight_dec(gp_backend *self)
{
	enum gp_backend_backlight_req req = GP_BACKEND_BACKLIGHT_DEC;

	if (self->set_attr)
		return self->set_attr(self, GP_BACKEND_ATTR_BACKLIGHT, &req);

	return GP_BACKEND_NOTSUPP;
}

/**
 * @brief Returns backlight intensity in percents.
 *
 * There are a few backends that can modify the backlight intensity, e.g. Linux
 * DRM.
 *
 * @param self A backend.
 * @return A backlight intensity in percents or GP_BACKEND_NOTSUPP if
 *         backlight is not supported by the backend.
 */
static inline int gp_backend_backlight_get(gp_backend *self)
{
	enum gp_backend_backlight_req req = GP_BACKEND_BACKLIGHT_GET;

	if (self->set_attr)
		return self->set_attr(self, GP_BACKEND_ATTR_BACKLIGHT, &req);

	return GP_BACKEND_NOTSUPP;
}

/**
 * @brief Exits the backend.
 *
 * This functions deinitializes the backend. E.g. closes all file descriptors,
 * frees memory, etc.
 *
 * @warning It's important to call this before the application exits since some
 *          backends, e.g. framebuffer, cannot be recovered unless we return
 *          the underlying facility to the original state.
 *
 * @param self A backend.
 */
void gp_backend_exit(gp_backend *self);

/**
 * @brief Polls a backend for events.
 *
 * This is a non-blocking function that fills the events into the backend event
 * queue.
 *
 * @param self A backend.
 */
void gp_backend_poll(gp_backend *self);

/**
 * @brief Returns an event from the queue, polls the backend for events if the
 *        queue is empty.
 *
 * This combines a gp_backend_poll() and gp_backend_ev_get() into a one
 * function. The call to gp_backend_poll() may generate more than one event so
 * the function tries to return events from the queue first and only if the
 * queue is empty it reaches to the backend to try to fill the queue.
 *
 * The function is supposed to be called in a loop:
 *
 * @code
 * gp_event *ev;
 *
 * while ((ev = gp_backend_ev_poll(backend)) {
 *	// Event processing has to be done here
 *
 *      // To avoid bussy loop we have to sleep here
 *	usleep(100);
 * }
 * @endcode
 *
 * @param self A backend.
 * @return An event or NULL if queue was empty and no events were generated by by backend.
 */
gp_event *gp_backend_ev_poll(gp_backend *self);

/**
 * @brief Waits for a backend events with a timeout.
 *
 * This is a blocking call that fills the events into the backend event queue
 * until timeout.
 *
 * Once this call returns the events from the event queue must be processed.
 *
 * The event processing may look like:
 *
 * @code
 * for (;;) {
 *	gp_event *ev;
 *
 *	gp_backend_wait_timeout(backend, 10);
 *
 *	while ((ev = gp_backend_ev_get(backend)) {
 *	// Event processing has to be done here
 *	}
 * }
 * @endcode
 *
 * @param self A backend.
 * @param timeout_ms A timeout in miliseconds. Passing -1 means no timeout.
 */
void gp_backend_wait_timeout(gp_backend *self, int timeout_ms);

/**
 * @brief Waits for a backend events.
 *
 * This is a blocking call that fills the events into the backend event queue.
 *
 * Once this call returns the events from the event queue must be processed.
 *
 * The event processing may look like:
 *
 * @code
 * for (;;) {
 *	gp_event *ev;
 *
 *	gp_backend_wait(backend);
 *
 *	while ((ev = gp_backend_ev_get(backend)) {
 *	// Event processing has to be done here
 *	}
 * }
 * @endcode
 *
 * @param self A backend.
 */
static inline void gp_backend_wait(gp_backend *self)
{
	gp_backend_wait_timeout(self, -1);
}

/**
 * @brief Returns an event from the queue, wait for the backend events until a
 *        timeout if the queue is empty.
 *
 * This combines a gp_backend_wait_timeout() and gp_backend_ev_get() into a one
 * function. The call to gp_backend_wait_timeout() may generate more than one
 * event so the function tries to return events from the queue first and only
 * if the queue is empty it reaches to the backend to try to fill the queue.
 *
 * The function is supposed to be called in a loop:
 *
 * @code
 * for (;;) {
 *	gp_event *ev = gp_backend_ev_wait_timeout(backend, 10);
 *
 *	// Event processing has to be done here
 * }
 * @endcode
 *
 * @param self A backend.
 * @param timeout_ms A timeout in miliseconds. Passing -1 means no timeout.
 * @return An event.
 */
gp_event *gp_backend_ev_wait_timeout(gp_backend *self, int timeout_ms);

/**
 * @brief Returns an event from the queue, wait the backend for events if the
 *        queue is empty.
 *
 * This combines a gp_backend_wait() and gp_backend_ev_get() into a one
 * function. The call to gp_backend_wait() may generate more than one event so
 * the function tries to return events from the queue first and only if the
 * queue is empty it reaches to the backend to try to fill the queue.
 *
 * The function is supposed to be called in a loop:
 *
 * @code
 * for (;;) {
 *	gp_event *ev = gp_backend_ev_wait(backend));
 *
 *	// Event processing has to be done here
 * }
 * @endcode
 *
 * @param self A backend.
 * @return An event.
 */
static inline gp_event *gp_backend_ev_wait(gp_backend *self)
{
	return gp_backend_ev_wait_timeout(self, -1);
}

/**
 * @brief Adds a timer to a backend timer queue.
 *
 * If timer callback is NULL a #gp_ev with #GP_EV_TMR type is pushed into the
 * backend event queue once timer expires.
 *
 * @param self A backend.
 * @param timer A timer.
 */
void gp_backend_timer_start(gp_backend *self, gp_timer *timer);

/**
 * @brief Reschedules a timer.
 *
 * If a timer is running (inserted into the timer queue) it's stopped before
 * it's started again.
 *
 * @param self A backend.
 * @param timer A timer.
 * @param expires_ms A new timeout for the timer in miliseconds.
 */
void gp_backend_timer_reschedule(gp_backend *self, gp_timer *timer, uint32_t expires_ms);

/**
 * @brief Removes timer from backend timer queue.
 *
 * @param self A backend.
 * @param timer A timer.
 */
void gp_backend_timer_stop(gp_backend *self, gp_timer *timer);

/**
 * @brief Returns number of timers scheduled in backend.
 *
 * @param self A backend.
 */
static inline unsigned int gp_backend_timers_queued(gp_backend *self)
{
	return gp_timer_queue_size(self->timers);
}

/**
 * @brief Returns time to the closest timer timeout.
 *
 * @param self A backend.
 *
 * @return Returns a timeout to a closest timer in ms or -1. Can be passed
 *         directly to poll(2).
 */
int gp_backend_timer_timeout(gp_backend *self);

/**
 * @brief Sets backend caption, if supported.
 *
 * Caption is an UTF8 string that is usually shown in the window title.
 *
 * @return On success #GP_BACKEND_OK is returned. When setting caption is not
 *         possible or implemented #GP_BACKEND_NOTSUPP is returned.
 */
static inline enum gp_backend_ret gp_backend_set_caption(gp_backend *backend,
                                                         const char *caption)
{
	if (!backend->set_attr)
		return GP_BACKEND_NOTSUPP;

	return backend->set_attr(backend, GP_BACKEND_ATTR_TITLE, caption);
}

/**
 * @brief Request backend resize.
 *
 * When the backend size matches the requested width and height no action is done.
 *
 * If a resize request is granted the backend sends #GP_EV_SYS event type with
 * #GP_EV_SYS_RESIZE code and the new width and height in the struct gp_ev_sys.
 * Upon receiving this event the application must stop any access to the
 * gp_backend::pixmap and call gp_backend_resize_ack() to signal to the backend
 * that it's safe to resize the buffers. Once that is done the application can
 * start drawing to the new and resized gp_backend::pixmap.
 *
 * @return When resizing is not possible or not implemented non zero is returned.
 */
int gp_backend_resize(gp_backend *backend, uint32_t w, uint32_t h);

/**
 * @brief Reuqests to change fullscreen mode.
 *
 * @param backend A backend.
 * @param req A fullscreen request.
 *
 * @return #GP_BACKEND_OK on success, negative on failure. In the case of
 *         #GP_BACKEND_FULLSCREEN_QUERY it's either #GP_BACKEND_ON,
 *         #GP_BACKEND_OFF or an error, most commonly #GP_BACKEND_NOTSUPP.
 */

static inline enum gp_backend_ret gp_backend_fullscreen(gp_backend *backend,
                                                        enum gp_backend_fullscreen_req req)
{
	if (!backend->set_attr)
		return GP_BACKEND_NOTSUPP;

	return backend->set_attr(backend, GP_BACKEND_ATTR_FULLSCREEN, &req);
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
 * This call also resizes the window, display, screen size in the backend
 * #gp_ev_queue.
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
 * @warning When backend is created there is no task queue, to be able to
 *          schedule tasks user has to allocate and set the task queue first.
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

/**
 * @brief Removes all events from the event queue.
 *
 * @param self A backend.
 */
static inline void gp_backend_ev_flush(gp_backend *self)
{
	return gp_ev_queue_flush(self->event_queue);
}

/**
 * @brief Returns a number of events in the backend event queue.
 *
 * @param self A backend.
 *
 * @return A number of events queued.
 */
static inline unsigned int gp_backend_ev_queued(gp_backend *self)
{
	return gp_ev_queue_events(self->event_queue);
}

/**
 * @brief Removes and returns a pointer to a first event in the queue.
 *
 * @warning The pointer to the event is valid only until the next call to a
 *          function that modifies the queue. That includes the functions that
 *          fill in the queue e.g.  gp_backend_wait().
 *
 * @param self A backend.
 * @return A pointer to an event or NULL if queue is empty.
 */
gp_event *gp_backend_ev_get(gp_backend *self);

/**
 * @brief Returns a pointer to a first event in the queue.
 *
 * Unlike the gp_backend_ev_get() the event is not removed from the queue.
 *
 * @warning The pointer to the event is valid only until the next call to a
 *          function that modifies the queue. That includes the functions that
 *          fill in the queue e.g.  gp_backend_wait().
 *
 * @param self A backend.
 * @return A pointer to an event or NULL if queue is empty.
 */
static inline gp_event *gp_backend_ev_peek(gp_backend *self)
{
	return gp_ev_queue_peek(self->event_queue);
}

/**
 * @brief Puts back an event that has been just removed from the queue.
 *
 * This can undo a last call to gp_backend_ev_get() by putting the event that
 * has been just removed back into the queue.
 *
 * @warning An attempt to put back more than one evevent or an event that
 *          wasn't removed from the top of the queue is not supported.
 *
 * @param self A backend.
 * @param ev An event to be put back into the queue.
 */
static inline void gp_backend_ev_put_back(gp_backend *self, gp_event *ev)
{
	gp_ev_queue_put_back(self->event_queue, ev);
}

#endif /* BACKENDS_GP_BACKEND_H */
