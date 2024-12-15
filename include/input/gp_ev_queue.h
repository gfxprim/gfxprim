// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_ev_queue.h
 * @brief Event Queue.
 *
 * This API is internally used by backends to store input events and is not
 * supposed to be used directly.
 */

#ifndef INPUT_GP_EV_QUEUE_H
#define INPUT_GP_EV_QUEUE_H

#include <input/gp_ev_feedback.h>
#include <input/gp_event.h>

/** @brief Maximal number of events the queue can hold. */
#define GP_EVENT_QUEUE_SIZE 32

/** @brief An event queue */
struct gp_ev_queue {
	/** @brief A display/window/screen width. */
	unsigned int screen_w;
	/** @brief A display/window/screen height. */
	unsigned int screen_h;

	/** @brief A cursor x position valid for the last event in the queue. */
	uint32_t cursor_x;
	/** @brief A cursor y position valid for the last event in the queue. */
	uint32_t cursor_y;

	/* event queue */
	unsigned int queue_first;
	unsigned int queue_last;
	unsigned int queue_size;

	/**
	 * @brief A keymap.
	 *
	 * This is needed only for backends that does not provide a keymap
	 * themselves. This includes framebuffer, Linux DRM, e-ink displays,
	 * etc.
	 */
	gp_keymap *keymap;

	/** @brief A list of callbacks to turn on/off leds, e.g. caps lock, and other feedback. */
	gp_ev_feedback *feedbacks_list;

	/**
	 * @brief An accumulated state.
	 *
	 * Maintains pressed keys, cursor position, etc.
	 *
	 * Valid only for event removed by the last call of gp_ev_queue_get().
	 */
	gp_events_state state;

	/** @brief A circular buffer for input events. */
	gp_event events[GP_EVENT_QUEUE_SIZE];
};

/** @brief An event queue init flags. */
enum gp_ev_queue_flags {
	/**
	 * @brief Load keymap during initialization.
	 *
	 * This is used by backends that do not get keystrokes translated to
	 * unicode characters, e.g. framebuffer, Linux DRM, e-ink displays,
	 * etc.
	 */
	GP_EVENT_QUEUE_LOAD_KEYMAP = 0x01,
};

/**
 * @brief Initializes event queue.
 *
 * The queue has to be allocatedpassed as a pointer. The events array must be
 * queue_size long.
 *
 * If queue_size is set to zero, default value is expected.
 *
 * @param self A pointer to newly allocated event queue structure.
 * @param screen_w A width of the display/window.
 * @param screen_h A height of the display/window.
 * @param queue_size A size of the circular buffer for the queue, pass 0 for default.
 * @param flags A bitwise combination of enum gp_ev_queue_flags.
 */
void gp_ev_queue_init(gp_ev_queue *self,
                      unsigned int screen_w, unsigned int screen_h,
                      unsigned int queue_size, enum gp_ev_queue_flags flags);


/**
 * @brief Removes all events from the queue.
 *
 * @param self An event queue.
 */
void gp_ev_queue_flush(gp_ev_queue *self);

/**
 * @brief Sets screen (window) size.
 *
 * This is needs to be called when backend is resized in order to update cursor
 * position and limits.
 *
 * @param self An event queue.
 * @param w New display/window/screen width.
 * @param h New display/window/screen height.
 */
void gp_ev_queue_set_screen_size(gp_ev_queue *self,
                                 unsigned int w, unsigned int h);

/**
 * @brief Registers an input device feedback callback.
 *
 * @param self An event queue.
 * @param feedback An input device feedback callback.
 */
static inline void gp_ev_queue_feedback_register(gp_ev_queue *self, gp_ev_feedback *feedback)
{
	gp_ev_feedback_register(&self->feedbacks_list, feedback);
}

/**
 * @brief Unregisters an input device feedback callback.
 *
 * @param self An event queue.
 * @param feedback An input device feedback callback.
 */
static inline void gp_ev_queue_feedback_unregister(gp_ev_queue *self, gp_ev_feedback *feedback)
{
	gp_ev_feedback_unregister(&self->feedbacks_list, feedback);
}

/**
 * @brief Calls a all feedback handlers with a specified value.
 *
 * @param self An event queue.
 * @param op An operation to be done, e.g. turn on caps lock led.
 */
static inline void gp_ev_queue_feedback_set_all(gp_ev_queue *self, gp_ev_feedback_op *op)
{
	gp_ev_feedback_set_all(self->feedbacks_list, op);
}

/**
 * @brief Sets a cursor postion.
 *
 * @param self An event queue.
 * @param x A new cursor x position.
 * @param y A new cursor y position.
 */
void gp_ev_queue_set_cursor_pos(gp_ev_queue *self,
                                unsigned int x, unsigned int y);

/**
 * @brief Returns number of events queued in the queue.
 *
 * @param self An event queue.
 * @return A number of events in the queue.
 */
unsigned int gp_ev_queue_events(gp_ev_queue *self);

/**
 * @brief Returns true if queue is full.
 *
 * @param self An event queue.
 * @return Non-zero if queue is full zero otherwise.
 */
static inline int gp_ev_queue_full(gp_ev_queue *self)
{
	unsigned int next = (self->queue_last + 1) % self->queue_size;

	return next == self->queue_first;
}

/**
 * @brief Remoes and returns pointer to a top event from the queue.
 *
 * In case there are any events queued a pointer to a top event in the queue is
 * returned. The pointer is valid until next call to gp_ev_queue_get().
 *
 * The pointer is also invalidated by a call to gp_ev_queue_put_back().
 *
 * If there are no events queued the call returns NULL.
 *
 * @param self An event queue.
 * @return A pointer to the top event in the queue or NULL if queue is empty.
 */
gp_event *gp_ev_queue_get(gp_ev_queue *self);

/**
 * @brief Peeks at a top event from the queue.
 *
 * Same as gp_ev_queue_get() but the event is not removed from the queue.
 * The pointer is valid until a call to gp_ev_queue_get().
 *
 * If there are no events queued the calll returns NULL.
 *
 * @param self An event queue.
 * @return A pointer to the top event in the queue or NULL if queue is empty.
 */
gp_event *gp_ev_queue_peek(gp_ev_queue *self);

/**
 * @brief Pushes an event to the event queue.
 *
 * This is bare call that just copies the event into the queue. Use the calls
 * below instead.
 *
 * @param self An input queue.
 * @param ev A pointer to an event.
 */
void gp_ev_queue_put(gp_ev_queue *self, gp_event *ev);

/**
 * @brief Pushes back an event to the event queue.
 *
 * Puts event to the top of the queue, i.e. this event is going to be returned
 * by a next call to gp_ev_queue_get().
 *
 * @param self An input queue.
 * @param ev A pointer to an event.
 */
void gp_ev_queue_put_back(gp_ev_queue *self, gp_event *ev);

/**
 * @brief Pushes a relative event that moves cursor by rx and ry.
 *
 * @param self An input queue.
 * @param rx A relative change in x coordinate.
 * @param ry A relative change in y coordinate.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_rel(gp_ev_queue *self,
                          int32_t rx, int32_t ry, uint64_t time);

/**
 * @brief Pushes a relative event that moves cursor to the point x, y.
 *
 * @param self An input queue.
 * @param x A x coordinate to move the cursor to.
 * @param y A y coordinate to move the cursor to.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_rel_to(gp_ev_queue *self,
                             uint32_t x, uint32_t y, uint64_t time);

/**
 * @brief Pushes an absolute event into the queue.
 *
 * The values are scaled between 0 and max value.
 *
 * @param self An input queue.
 * @param x An absolute x coordinate.
 * @param y An absolute y coordinate.
 * @param pressure A pen pressure.
 * @param x_max An absolute x coordinate maximum.
 * @param y_max An absolute y coordinate maximum.
 * @param pressure_max A maximal pen pressure.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_abs(gp_ev_queue *self,
                          uint32_t x, uint32_t y, uint32_t pressure,
                          uint32_t x_max, uint32_t y_max, uint32_t pressure_max,
                          uint64_t time);

/**
 * @brief Pushes a key event into the event queue.
 *
 * Pushes a #GP_EV_KEY event to the event queue.
 *
 * If backend uses a keymap key events are processed by the keymap state
 * machine and #GP_EV_UTF events are generated accordingly.
 *
 * @param self An input queue.
 * @param key Physical key pressed on keyboard.
 * @param code Action enum #gp_ev_key_code press/release/repeat.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_key(gp_ev_queue *self,
                          uint32_t key, uint8_t code,
                          uint64_t time);

/**
 * @brief Pushes an unicode character typed on keyboard into the queue.
 *
 * @param self An input queue.
 * @param utf_ch An unicode character.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_utf(gp_ev_queue *self, uint32_t utf_ch,
                          uint64_t time);

/**
 * @brief Pushes a window resize event to the queue.
 *
 * Backends start the resize operation by pushing the #GP_EV_SYS_RESIZE event
 * to the input queue. When the application processes the event it has to
 * acknowledge the resize by calling gp_backend_resize_ack(), which resizes the
 * pixel buffers and also calls gp_ev_queue_set_screen_size().
 *
 * @param self An input queue.
 * @param new_w A new display/window/screen width.
 * @param new_h A new display/window/screen height.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push_resize(gp_ev_queue *self,
                             uint32_t new_w, uint32_t new_h, uint64_t time);

/**
 * @brief Push a generic event into the queue.
 *
 * @param self An input queue.
 * @param type An event type enum #gp_ev_type.
 * @param code Based on type one of the enum #gp_ev_key_code,
 *             enum #gp_ev_rel_code, enum #gp_ev_abs_code,
 *             enum #gp_ev_sys_code.
 * @param value An event value, e.g. which key was pressed.
 * @param time A timestamp, if NULL current time is used.
 */
void gp_ev_queue_push(gp_ev_queue *self,
                      uint16_t type, uint32_t code, int32_t value,
                      uint64_t time);

/**
 * @brief Pushes a mouse wheel movement into the queue.
 *
 * @param self An input queue.
 * @param val A relative change of the mouse wheel position.
 * @param time A timestamp, if NULL current time is used.
 */
static inline void gp_ev_queue_push_wheel(gp_ev_queue *self,
                                          int32_t val, uint64_t time)
{
	gp_ev_queue_push(self, GP_EV_REL, GP_EV_REL_WHEEL, val, time);
}

#endif /* INPUT_GP_EV_QUEUE_H */
