//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

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
 * Describes borders
 */
enum gp_widget_border {
	GP_WIDGET_BORDER_NONE = 0x00,

	GP_WIDGET_BORDER_LEFT = 0x01,
	GP_WIDGET_BORDER_RIGHT = 0x2,
	GP_WIDGET_BORDER_TOP = 0x10,
	GP_WIDGET_BORDER_BOTTOM = 0x20,

	GP_WIDGET_BORDER_HORIZ = GP_WIDGET_BORDER_TOP | GP_WIDGET_BORDER_BOTTOM,
	GP_WIDGET_BORDER_VERT = GP_WIDGET_BORDER_LEFT | GP_WIDGET_BORDER_RIGHT,

	GP_WIDGET_BORDER_ALL = GP_WIDGET_BORDER_HORIZ | GP_WIDGET_BORDER_VERT,

	/* clears all borders missing from the bitflags */
	GP_WIDGET_BORDER_CLEAR = 0x80
};

#endif /* GP_WIDGET_TYPES_H */
