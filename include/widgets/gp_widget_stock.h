//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_STOCK_H
#define GP_WIDGET_STOCK_H

#include <widgets/gp_widget_size_units.h>

enum gp_widget_stock_type {
	/* Messages */
	GP_WIDGET_STOCK_ERR,
	GP_WIDGET_STOCK_WARN,
	GP_WIDGET_STOCK_INFO,
	GP_WIDGET_STOCK_QUESTION,

	/* Audio */
	GP_WIDGET_STOCK_SPEAKER_MUTE,
	GP_WIDGET_STOCK_SPEAKER_MIN,
	GP_WIDGET_STOCK_SPEAKER_MID,
	GP_WIDGET_STOCK_SPEAKER_MAX,
	GP_WIDGET_STOCK_SPEAKER_INC,
	GP_WIDGET_STOCK_SPEAKER_DEC,

	/* Misc */
	GP_WIDGET_STOCK_HARDWARE,
	GP_WIDGET_STOCK_SOFTWARE,
	GP_WIDGET_STOCK_SETTINGS,
	GP_WIDGET_STOCK_SAVE,
	GP_WIDGET_STOCK_FILE,
	GP_WIDGET_STOCK_DIR,

	GP_WIDGET_STOCK_REFRESH,

	GP_WIDGET_STOCK_SHUFFLE_ON,
	GP_WIDGET_STOCK_SHUFFLE_OFF,

	/* Arrows */
	GP_WIDGET_STOCK_ARROW_UP,
	GP_WIDGET_STOCK_ARROW_DOWN,
	GP_WIDGET_STOCK_ARROW_LEFT,
	GP_WIDGET_STOCK_ARROW_RIGHT,

	/* Flags */
	GP_WIDGET_STOCK_FOCUSED = 0x8000,
};

#define GP_WIDGET_STOCK_TYPE_MASK (~(GP_WIDGET_STOCK_FOCUSED))
#define GP_WIDGET_STOCK_TYPE(type) ((type) & GP_WIDGET_STOCK_TYPE_MASK)

struct gp_widget_stock {
	enum gp_widget_stock_type type;
	gp_widget_size min_size;
};

void gp_widget_stock_render(gp_pixmap *pix, enum gp_widget_stock_type type,
                            gp_coord x, gp_coord y, gp_size w, gp_size h,
                            gp_pixel bg_col, const gp_widget_render_ctx *ctx);

gp_widget *gp_widget_stock_new(enum gp_widget_stock_type type, gp_widget_size min_size);

void gp_widget_stock_type_set(gp_widget *self, enum gp_widget_stock_type type);

#endif /* GP_WIDGET_STOCK_H */
