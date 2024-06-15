//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_types.h
 * @brief Common widget types.
 */
#ifndef GP_WIDGET_TYPES_H
#define GP_WIDGET_TYPES_H

#include <stdint.h>

typedef struct gp_widget_event gp_widget_event;
typedef struct gp_widget gp_widget;
typedef struct gp_dialog gp_dialog;
typedef struct gp_widget_size gp_widget_size;
typedef struct gp_widget_render_ctx gp_widget_render_ctx;
typedef struct gp_widget_json_ctx gp_widget_json_ctx;
typedef struct gp_widget_json_callbacks gp_widget_json_callbacks;

typedef uint32_t gp_widget_tattr;

/**
 * @brief Describes borders.
 */
enum gp_widget_border {
	/** @brief No border selected. */
	GP_WIDGET_BORDER_NONE = 0x00,
	/** @brief Left border selected. */
	GP_WIDGET_BORDER_LEFT = 0x01,
	/** @brief Right border selected. */
	GP_WIDGET_BORDER_RIGHT = 0x2,
	/** @brief Top border selected. */
	GP_WIDGET_BORDER_TOP = 0x10,
	/** @brief Bottom border selected. */
	GP_WIDGET_BORDER_BOTTOM = 0x20,
	/** @brief Horizontal borders selected. */
	GP_WIDGET_BORDER_HORIZ = GP_WIDGET_BORDER_TOP | GP_WIDGET_BORDER_BOTTOM,
	/** @brief Vertical borders selected. */
	GP_WIDGET_BORDER_VERT = GP_WIDGET_BORDER_LEFT | GP_WIDGET_BORDER_RIGHT,
	/** @brief All borders selected. */
	GP_WIDGET_BORDER_ALL = GP_WIDGET_BORDER_HORIZ | GP_WIDGET_BORDER_VERT,
	/**
	 * @brief Clear borders not selected in bitmask.
	 *
	 * Clears (sets to 0) all borders not from the bitflags, when passed to
	 * a function that sets borders, e.g. gp_widget_grid_border_set().
	 */
	GP_WIDGET_BORDER_CLEAR = 0x80
};

#endif /* GP_WIDGET_TYPES_H */
