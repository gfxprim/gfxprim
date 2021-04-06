//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_STOCK_H
#define GP_WIDGET_STOCK_H

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
};

struct gp_widget_stock {
	enum gp_widget_stock_type type;
	unsigned int min_size;
};

gp_widget *gp_widget_stock_new(enum gp_widget_stock_type type, int min_size);

void gp_widget_stock_type_set(gp_widget *self, enum gp_widget_stock_type type);

#endif /* GP_WIDGET_STOCK_H */
