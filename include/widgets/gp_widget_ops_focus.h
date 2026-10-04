//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_ops_focus.h
 * @brief Widget internals.
 *
 * Implements internal functions to handle widget focus.
 */

#ifndef GP_WIDGET_OPS_FOCUS_H
#define GP_WIDGET_OPS_FOCUS_H

#include <input/gp_event.h>
#include <utils/gp_bbox.h>
#include <utils/gp_json.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_render.h>

/** @brief Flags to move focus around. */
enum gp_widget_focus_flag {
	/** @brief Moves focus left. */
	GP_FOCUS_LEFT,
	/** @brief Moves focus right. */
	GP_FOCUS_RIGHT,
	/** @brief Moves focus up. */
	GP_FOCUS_UP,
	/** @brief Moves focus down. */
	GP_FOCUS_DOWN,
	/** @brief Focus next widget. */
	GP_FOCUS_NEXT,
	/** @brief Focus previous widget. */
	GP_FOCUS_PREV,
};

/**
 * @brief Checks if a widget is focusable.
 *
 * A widget, in order to be focusable, has to be able to process input events,
 * i.e. the widget event handler has to be implemented and enabled. At the same
 * time focus cannot be moved to a disabled widget.
 *
 * @param self A widget.
 * @return True if widget is focusable.
 */
bool gp_widget_focusable(gp_widget *self);

/**
 * @brief Checks if a widget is focused.
 *
 * A focused widget is the widget that ends up processing input events, i.e.
 * last widget in the path from widget root with the focused flag enabled.
 *
 * @param self A widget.
 * @return True if widget is last widget in the focused path from layout root.
 */
bool gp_widget_focused(gp_widget *self);

/**
 * @brief Moves focus, if possible, in the direction requested by the focus_dir.
 *
 * @param self A widget layout.
 * @param focus_dir Direction to move the focus to.
 *
 * @return Zero if focus couldn't be moved, non-zero otherwise.
 */
int gp_widget_ops_render_focus(gp_widget *self, int focus_dir);

/**
 * @brief Removes the focus from a layout.
 *
 * Unfocuses all widgets on the focus path. The focused children are
 * remembered by the containers so that moving the focus into the layout
 * again moves it back where it was.
 *
 * @param self A widget layout.
 *
 * @return Zero if nothing was focused, non-zero otherwise.
 */
int gp_widget_ops_focus_out(gp_widget *self);

/**
 * @brief Tries to focus a widget on a given coordinates in a layout.
 *
 * @param self A widget layout.
 * @param ctx A render context.
 * @param x X coordinate.
 * @param y Y coordinate.
 *
 * @return Zero if focus wasn't changed, non-zero otherwise.
 */
int gp_widget_ops_render_focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                                  unsigned int x, unsigned int y);

/**
 * @brief Fixes the focus after a container changed its focused child.
 *
 * Has to be called by container widgets after the child the container stores
 * as focused changed for reasons other than focus, e.g. a tab was switched, a
 * layer was hidden or a child was removed, and when the focused child cannot
 * be focused anymore, i.e. it was disabled. The container state has to be
 * updated before this function is called and the old child must not be freed
 * yet.
 *
 * If the old child had focus, it's unfocused and the focus moves into the
 * new focused child, to the next or the previous widget in the container, to
 * the container itself if it can be focused, or elsewhere in the layout.
 *
 * @param self A container widget.
 * @param old_child A child that was focused before the change, may be NULL.
 */
void gp_widget_ops_focus_child_changed(gp_widget *self, gp_widget *old_child);

/**
 * @brief Moves focus to a particular widget.
 *
 * Traverses the widget layout tree to the top and sets the focus accordingly.
 *
 * @param self A widget to be focused.
 *
 * @return Zero if focus couldn't be changed, non-zero otherwise.
 */
int gp_widget_ops_focus_widget(gp_widget *self);

#endif /* GP_WIDGET_OPS_H */
