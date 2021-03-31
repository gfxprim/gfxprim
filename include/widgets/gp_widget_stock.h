//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_STOCK_H
#define GP_WIDGET_STOCK_H

enum gp_widget_stock_types {
	GP_WIDGET_STOCK_ERR,
	GP_WIDGET_STOCK_WARN,
	GP_WIDGET_STOCK_INFO,
	GP_WIDGET_STOCK_QUESTION,
};

gp_widget *gp_widget_stock_new(enum gp_widget_stock_types type);

#endif /* GP_WIDGET_STOCK_H */
