//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_stock_switch.h
 * @brief A stock switch widget.
 *
 * Stock switch is a widget that cycles between two images to represent on and
 * off state and is changed by clicking on the stock images. Consider for example
 * shuffle_on and shuffle_off stock images as a canonical example of such widget.
 *
 * Stock switch is a bool class widget, after it's created it's acessed by the
 * class functions such as gp_widget_bool_set().
 *
 * Stock switch widget JSON attributes
 * -----------------------------------
 *
 * |   Attribute   |  Type  |       Default        | Description                                          |
 * |---------------|--------|----------------------|------------------------------------------------------|
 * |  **min_size** | string |     2*pad + asc      | Minimal size parsed by gp_widget_size_units_parse(). |
 * |  **on_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when switch is on.                       |
 * | **off_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when switch is off.                      |
 * |    **set**    |  bool  |         false        | Initial switch state.                                |
 */

#ifndef GP_WIDGET_STOCK_SWITCH_H
#define GP_WIDGET_STOCK_SWITCH_H

#include <widgets/gp_widget_class_bool.h>
#include <widgets/gp_widget_stock.h>

/**
 * @brief Allocate and initialize new stock switch widget.
 *
 * @param on_stock A stock image when switch is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_stock A stock image when switch is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param min_size A minimal widget size parsed by
 *                 gp_widget_size_units_parse(). Pass
 *                 GP_WIDGET_SIZE_DEFAULT for a default size.
 * @param set An initial state for the switch.
 *
 * @return A stock switch widget.
 */
gp_widget *gp_widget_stock_switch_new(gp_widget_stock_type on_stock,
                                      gp_widget_stock_type off_stock,
                                      gp_widget_size min_size, bool set);

/**
 * @brief Allocate and initialize new stock switch widget.
 *
 * @param on_stock A stock image when switch is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_stock A stock image when switch is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param set An initial state for the switch.
 * @param on_event An widget event handler.
 * @param priv User private data.
 *
 * @return A stock switch widget.
 */
static inline gp_widget *gp_widget_stock_switch_new_ev(gp_widget_stock_type on_stock,
                                                       gp_widget_stock_type off_stock,
				                       gp_widget_size min_size,
						       bool set,
                                                       int (*on_event)(gp_widget_event *ev),
                                                       void *priv)
{
	gp_widget *ret = gp_widget_stock_switch_new(on_stock, off_stock, min_size, set);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

#endif /* GP_WIDGET_STOCK_SWITCH_H */
