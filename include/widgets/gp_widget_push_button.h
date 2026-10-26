//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_push_button.h
 * @brief A push button widget.
 *
 * Push button is a bool class widget that renders like a button but keeps an
 * on/off state, i.e. it stays pushed in when it's on. After it's created it's
 * accessed by the class functions such as gp_widget_bool_set().
 *
 * Push button widget JSON attributes
 * ----------------------------------
 *
 * |   Attribute   |  Type  |       Default        | Description                       |
 * |---------------|--------|----------------------|-----------------------------------|
 * |   **label**   | string |         NULL         | A button label.                   |
 * |  **on_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when button is on.    |
 * | **off_stock** | string | GP_WIDGET_STOCK_NONE | Stock image when button is off.   |
 * |    **set**    |  bool  |         false        | Initial push button state.        |
 */

#ifndef GP_WIDGET_PUSH_BUTTON_H
#define GP_WIDGET_PUSH_BUTTON_H

#include <widgets/gp_widget_class_bool.h>
#include <widgets/gp_widget_stock.h>

/**
 * @brief Allocate and initialize new push button widget.
 *
 * @param label A button label, can be NULL if there is none.
 * @param on_stock A stock image when button is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_stock A stock image when button is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param set An initial state for the push button.
 *
 * @return A push button widget.
 */
gp_widget *gp_widget_push_button_new(const char *label,
                                     gp_widget_stock_type on_stock,
                                     gp_widget_stock_type off_stock,
                                     bool set);

/**
 * @brief Allocate and initialize new push button widget.
 *
 * @param label A button label, can be NULL if there is none.
 * @param on_stock A stock image when button is on, can be
 *                 GP_WIDGET_STOCK_NONE if there is none.
 * @param off_stock A stock image when button is off, can be
 *                  GP_WIDGET_STOCK_NONE if there is none.
 * @param set An initial state for the push button.
 * @param on_event An widget event handler.
 * @param priv An user private pointer.
 *
 * @return A push button widget.
 */
static inline gp_widget *gp_widget_push_button_new_ev(const char *label,
                                                      gp_widget_stock_type on_stock,
                                                      gp_widget_stock_type off_stock,
                                                      bool set,
                                                      int (*on_event)(gp_widget_event *ev),
                                                      void *priv)
{
	gp_widget *ret = gp_widget_push_button_new(label, on_stock, off_stock, set);

	gp_widget_on_event_set(ret, on_event, priv);

	return ret;
}

/**
 * @brief Sets a push button label.
 *
 * @param self A push button widget.
 * @param label A new label, can be NULL to remove the label.
 */
void gp_widget_push_button_label_set(gp_widget *self, const char *label);

/**
 * @brief Returns a push button label.
 *
 * @param self A push button widget.
 *
 * @return A push button label, may be NULL if there is none.
 */
const char *gp_widget_push_button_label_get(gp_widget *self);

#endif /* GP_WIDGET_PUSH_BUTTON_H */
