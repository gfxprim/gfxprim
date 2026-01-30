// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_event.h
 * @brief An (input) event layer.
 *
 * Input events combine events from HID devices, such as keyboard or mouse as
 * well as other types of events such as timers, window resize, clipboard, etc.
 */

#ifndef INPUT_GP_EVENT_H
#define INPUT_GP_EVENT_H

#include <stdint.h>
#include <string.h>

#include <utils/gp_types.h>
#include <input/gp_types.h>
#include <input/gp_keys.h>
#include <input/gp_event_keys.h>

/** @brief An event type. */
enum gp_ev_type {
	/** @brief A key or button press event. */
	GP_EV_KEY = 1,
	/** @brief A relative event, e.g. mouse cursor movement. */
	GP_EV_REL = 2,
	/** @brief An absolute event, e.g. touchscreen touch. */
	GP_EV_ABS = 3,
	/** @brief A system events, window close, resize, etc. */
	GP_EV_SYS = 4,
	/** @brief A timer expired. */
	GP_EV_TMR = 5,
	/** @brief A poll event on a filescriptor. */
	GP_EV_FD = 6,
	/** @brief Last used event type. */
	GP_EV_MAX = 6,
};

/** @brief A key event type. */
enum gp_ev_key_code {
	/** @brief Key released. */
	GP_EV_KEY_UP     = 0,
	/** @brief Key pressed. */
	GP_EV_KEY_DOWN   = 1,
	/**
	 * @brief Key repeated.
	 *
	 * Key is pressed for some time and repeat event is generated.
	 */
	GP_EV_KEY_REPEAT = 2,
};

/** @brief A relative event type. */
enum gp_ev_rel_code {
	/** @brief A relative position. */
	GP_EV_REL_POS   = 0,
	/** @brief A mouse wheel. */
	GP_EV_REL_WHEEL = 8,
};

/** @brief An absolute event type. */
enum gp_ev_abs_code {
	/** @brief An absolute position. */
	GP_EV_ABS_POS = 0,
};

/** @brief System events. */
enum gp_ev_sys_code {
	/**
	 * @brief User requested application to quit.
	 *
	 * This event is usually generated when user clicks on the button to
	 * close an application window.
	 */
	GP_EV_SYS_QUIT = 0,
	/**
	 * @brief User resized the applicaton window.
	 *
	 * This event has to be handled and the resize has to be acked by
	 * gp_backend_resize_ack() in order to resize the backend pixmap to
	 * match the window size.
	 */
	GP_EV_SYS_RESIZE = 1,
	/**
	 * @brief Clipboard request is ready.
	 *
	 * When application requests clipboard data by
	 * gp_backend_clipboard_request() the data are fetched and this event
	 * arrives to the appliction once the data are ready.
	 */
	GP_EV_SYS_CLIPBOARD = 2,
	/**
	 * @brief Backlight brightness change.
	 *
	 * When backend brightness was changed the new brightness in percents
	 * is stored in the val.
	 */
	GP_EV_SYS_BACKLIGHT = 3,

	/**
	 * @brief A focus in or focus out event.
	 *
	 * The event val is set to enum gp_ev_sys_focus_val.
	 */
	GP_EV_SYS_FOCUS = 4,

	/**
	 * @brief An application is shown on or hidden from a screen.
	 *
	 * The event val is set to enum gp_ev_sys_visibility_val
	 */
	GP_EV_SYS_VISIBILITY = 5,
};

/**
 * @brief Values for the GP_EV_SYS_FOCUS event.
 *
 * Application starts in focused out state and should expect to be notified
 * when it gains focus.
 */
enum gp_ev_sys_focus_val {
	/** @brief Application was focused out. */
	GP_EV_SYS_FOCUS_OUT = 0,
	/** @brief Appliaction was focused in. */
	GP_EV_SYS_FOCUS_IN = 1,
};

/**
 * @brief Values for the GP_EV_SYS_VISIBILITY event.
 *
 * Application should assume that it's visible unless it gets a hide event.
 *
 * When application is hidden it should stop rendering into the buffer to save
 * CPU cycles.
 */
enum gp_ev_sys_visibility_val {
	/** @brief Application is hidden from a screen. */
	GP_EV_SYS_VISIBILITY_HIDE = 0,
	/** @brief Application is shown on a screen. */
	GP_EV_SYS_VISIBILITY_SHOW = 1,
};

/** @brief A relative cursor position change. */
struct gp_ev_pos_rel {
	int32_t rx;
	int32_t ry;
};

/** @brief An absolute cursor position. */
struct gp_ev_pos_abs {
	uint32_t x, x_max;
	uint32_t y, y_max;
	uint32_t pressure, pressure_max;
};

/**
 * @brief A key press/release/repeat event.
 */
struct gp_ev_key {
	/**
	 * @brief One of enum gp_event_key_value.
	 *
	 * This is the raw key scancode. It can be set to zero if this is a
	 * synthetic unicode character.
	 */
	uint32_t key;
	/**
	 * @brief Unicode character (finalized) by this keystroke.
	 *
	 * If non-zero the key press was translated into a character by current
	 * keyboard mapping.
	 */
	uint32_t utf;
};

/** @brief An system event value. */
struct gp_ev_sys {
	/** @brief New width for a window resize event. */
	uint32_t w;
	/** @brief New height for a window resize event. */
	uint32_t h;
};

#define GP_EVENT_KEY_BITMAP_BYTES 56

/**
 * @brief An input events state.
 *
 * Tracks keys pressed at a given moment and a cursor position.
 */
struct gp_events_state {
	/** @brief Bitmap of pressed keys including mouse buttons. */
	uint8_t keys_pressed[GP_EVENT_KEY_BITMAP_BYTES];

	/** @brief Current cursor x coordinate. */
	uint32_t cursor_x;
	/** @brief Current cursor y coordinate. */
	uint32_t cursor_y;

	/** @brief Saved cursor x coordinate. */
	uint32_t saved_cursor_x;
	/** @brief Saved cursor y coordinate. */
	uint32_t saved_cursor_y;
};

/**
 * @brief An input event.
 */
struct gp_event {
	/** @brief enum gp_event_type  */
	uint16_t type;
	/** @brief enum gp_event_*_code */
	uint16_t code;
	/** Input event values. */
	union {
		/** @brief A generic integer value. */
		int32_t val;
		/** @brief A keyboard or mouse key. */
		struct gp_ev_key key;
		/** @brief A relative change in position. */
		struct gp_ev_pos_rel rel;
		/** @brief An absolute position. */
		struct gp_ev_pos_abs abs;
		/** @brief A system event. */
		struct gp_ev_sys sys;
		/** @brief A timer expired event. */
		gp_timer *tmr;
		/** @brief A poll fd event. */
		gp_fd *fd;
	};

	/** @brief An event timestamp. */
	uint64_t time;

	/** @brief Pointer to overall state, pressed keys, cursor position etc. */
	struct gp_events_state *st;
};

/**
 * @brief Checks if a character is not printable.
 *
 * @param ev A key input event.
 * @return True if utf character is a control character (non-printable).
 */
static inline int gp_ev_utf_is_ctrl(const gp_event *ev)
{
	if (!ev->key.utf)
		return 0;

	return ev->key.utf < 0x20 && ev->key.utf != 0x7f;
}

/**
 * @brief Helper to set key bit in the event state.
 *
 * @param self An event state.
 * @param key A key value to press.
 */
static inline void gp_events_state_press(gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] |= 1<<((key)%8);
}

/**
 * @brief Helper to get key bit from the event state.
 *
 * @param self An event state.
 * @param key A key value to get the state for.
 *
 * @return True if the key pressed bit is set in the state keys bitmap.
 */
static inline int gp_events_state_pressed(const gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return 0;

	return !!(self->keys_pressed[(key)/8] & (1<<((key)%8)));
}

/**
 * @brief Helper to reset key bit in the event state.
 *
 * @param self An event state.
 * @param key A key value to release.
 */
static inline void gp_events_state_release(gp_events_state *self, uint32_t key)
{
	if (key >= GP_EVENT_KEY_BITMAP_BYTES * 8)
		return;

	self->keys_pressed[(key)/8] &= ~(1<<((key)%8));
}

/**
 * @brief Helper to reset all key bits in the event state.
 *
 * @param self An event state.
 */
static inline void gp_events_state_release_all(gp_events_state *self)
{
	memset(self->keys_pressed, 0, sizeof(self->keys_pressed));
}

/**
 * @brief Checks if key is being pressed.
 *
 * @param ev An input event.
 * @param key A key value to check for.
 *
 * @return True if key is being pressed.
 */
static inline int gp_ev_key_pressed(const gp_event *ev, uint32_t key)
{
	if (!ev->st)
		return 0;

	return gp_events_state_pressed(ev->st, key);
}

/**
 * @brief Checks for any key from a set to be pressed.
 *
 * @param ev An input event.
 * @param ... A list of key values to check for.
 *
 * @return True if any key from a list is pressed.
 */
#define gp_ev_any_key_pressed(ev, ...) gp_ev_any_key_pressed_(ev, __VA_ARGS__, 0)
int gp_ev_any_key_pressed_(const gp_event *ev, ...);

/**
 * @brief Checks for all key from a set to be pressed.
 *
 * @param ev An input event.
 * @param ... A list of key values to check for.
 * @return True if all keys from a list are pressed.
 */
#define gp_ev_all_keys_pressed(ev, ...) gp_ev_all_keys_pressed_(ev, __VA_ARGS__, 0)
int gp_ev_all_keys_pressed_(const gp_event *ev, ...);

/**
 * @brief Dumps event into a stdout.
 *
 * @param ev A pointer to an input event.
 */
void gp_ev_dump(const gp_event *ev);

#endif /* INPUT_GP_EVENT_H */
