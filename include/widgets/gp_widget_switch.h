//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_switch.h
 * @brief A switch widget.
 *
 * Switch is a bool class widget, after it's created it's acessed by the
 * class functions such as gp_widget_bool_set().
 *
 * Switch widget JSON attributes
 * -----------------------------
 *
 * |   Attribute   |  Type  |       Default        | Description                       |
 * |---------------|--------|----------------------|---------------00------------------|
 * |   **label**   | string |         NULL         | Sets both off_label and on_label. |
 * |  **on_label** | string |         NULL         | A label when switch is on.        |
 * |  **on_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when switch is on.    |
 * | **off_label** | string |         NULL         | A label when switch is off.       |
 * | **off_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when switch is off.   |
 * |    **set**    |  bool  |         false        | Initial switch state.             |
 */

#ifndef GP_WIDGET_SWITCH_H
#define GP_WIDGET_SWITCH_H

#include <widgets/gp_widget_class_bool.h>
#include <widgets/gp_widget_stock.h>

/**
 * @brief Allocate and initialize new switch widget.
 *
 * @param on_label A label when switch is on, can be NULL if there is none.
 * @param on_stock A stock image when switch is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_label A label when switch is off, can be NULL if there is none.
 * @param off_stock A stock image when switch is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param set An initial state for the switch.
 *
 * @return A switch widget.
 */
gp_widget *gp_widget_switch_new(const char *on_label, gp_widget_stock_type on_stock,
                                const char *off_label, gp_widget_stock_type off_stock,
				bool set);

/**
 * @brief Allocate and initialize new switch widget.
 *
 * @param on_label A label when switch is on, can be NULL if there is none.
 * @param on_stock A stock image when switch is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_label A label when switch is off, can be NULL if there is none.
 * @param off_stock A stock image when switch is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param set An initial state for the switch.
 * @param on_event An widget event handler.
 * @param priv User private data.
 *
 * @return A switch widget.
 */
static inline gp_widget *gp_widget_switch_new_ev(const char *on_label,
                                                 gp_widget_stock_type on_stock,
                                                 const char *off_label,
                                                 gp_widget_stock_type off_stock,
				                 bool set,
                                                 int (*on_event)(gp_widget_event *ev),
                                                 void *priv)
{
	gp_widget *ret = gp_widget_switch_new(on_label, on_stock, off_label, off_stock, set);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

#endif /* GP_WIDGET_SWITCH_H */
