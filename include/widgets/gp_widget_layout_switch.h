//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_LAYOUT_SWITCH_H
#define GP_WIDGET_LAYOUT_SWITCH_H

/**
 * @brief Allocates a initializes a switch widget.
 *
 * @layouts Number of layouts to switch between.
 *
 * @return A switch widget.
 */
gp_widget *gp_widget_layout_switch_new(unsigned int layouts);

/**
 * @brief Returns number of switch layouts.
 *
 * @self A switch widget.
 *
 * @return Number of switch layouts.
 */
unsigned int gp_widget_layout_switch_layouts(gp_widget *self);

/**
 * @brief Switches to a different layout.
 *
 * @self A switch widget.
 * @layout_nr Number of layout to switch to.
 */
void gp_widget_layout_switch_layout(gp_widget *self, unsigned int layout_nr);

/**
 * @brief Moves layout by where.
 *
 * @self A switch widget.
 * @where How much we should move.
 */
void gp_widget_layout_switch_move(gp_widget *self, int where);

/**
 * @brief Returns a pointer to active layout.
 *
 * @self A switch widget.
 * @return A pointer to active layout.
 */
gp_widget *gp_widget_layout_switch_active(gp_widget *self);

/**
 * @brief Sets a switch layout.
 *
 * @self A switch widget.
 * @layout_nr Number of layout to set.
 * @child A child widget to set.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_layout_switch_put(gp_widget *self, unsigned int layout_nr,
                                       gp_widget *child);

#endif /* GP_WIDGET_LAYOUT_SWITCH_H */
