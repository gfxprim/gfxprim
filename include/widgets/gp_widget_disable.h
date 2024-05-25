//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_disable.h
 * @brief Functions to disable and enable widgets.
 *
 * Disabled widgets are widgets that does not react to user input and are
 * "grayed out" in the GUI. Once widget is disabled all its subwidgets are disabled as
 * well.
 *
 * Disabled widget example
 * -----------------------
 *
 * @include{c} demos/widgets/disable.c
 * @image html widgets_disable.png
 */

#ifndef GP_WIDGET_DISABLE_H
#define GP_WIDGET_DISABLE_H

#include <widgets/gp_widget_types.h>

/**
 * @brief Disables widget and all its subwidgets.
 *
 * A disabled widget does not process any input events and is "grayed out".
 *
 * @param self A widget to disable.
 */
void gp_widget_disable(gp_widget *self);

/**
 * @brief Enables widget and all its subwidgets.
 *
 * Enables a widget that has been disabled previously.
 *
 * @param self A widget to enable.
 */
void gp_widget_enable(gp_widget *self);

/**
 * @brief Sets disabled/enabled widget state.
 *
 * A disabled widget does not process any input events and is "grayed out".
 *
 * @param self A widget.
 * @param disabled True to disable widget false to enable it.
 */
void gp_widget_disabled_set(gp_widget *self, bool disabled);

/**
 * @brief Returns true if widget is disabled.
 *
 * Note that this function returns true only if particular widget is disabled
 * explicitly. It will return false for child widgets that are disabled because
 * of parent widgets have been disabled.
 *
 * @param self A widget.
 * @return True if widget is disabled.
 */
bool gp_widget_disabled_get(gp_widget *self);

#endif /* GP_WIDGET_DISABLE_H */
