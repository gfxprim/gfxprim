//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_OVERLAY_H
#define GP_WIDGET_OVERLAY_H

/*
 * Overlay widget is a widget that allows widgets to be stack on the top of
 * each other. The main use for this widget are all kinds of floating
 * menus/dialogs etc.
 */

struct gp_widget_overlay_elem {
	int hidden:1;
	gp_widget *widget;
};

struct gp_widget_overlay {
	int focused;
	struct gp_widget_overlay_elem *stack;
};

/**
 * @brief Allocates new overlay widget.
 *
 * @stack_size Initial size for the overlay stack size.
 *
 * @return Newly allocated overlay widget.
 */
gp_widget *gp_widget_overlay_new(unsigned int stack_size);

/**
 * @brief Hides a widget in overlay stack.
 *
 * @self An overlay widget.
 * @stack_pos Position of a widget to hide.
 */
void gp_widget_overlay_hide(gp_widget *self, unsigned int stack_pos);

/**
 * @brief Shows a widget in overlay stack.
 *
 * @self An overlay widget.
 * @stack_pos Position of a widget to hide.
 */
void gp_widget_overlay_show(gp_widget *self, unsigned int stack_pos);

/**
 * @brief Returns the widget overlay stack size.
 *
 * @self An overlay widget.
 * @return Stack size.
 */
unsigned int gp_widget_overlay_stack_size(gp_widget *self);

/**
 * @brief Returns widget overlay stack position by child widget pointer.
 *
 * @self An overlay widget.
 * @child An overlay widget child widget.
 * @stack_pos A variable to store the stack position to.
 * @return Zero-on succes, non-zero if not found.
 */
int gp_widget_overlay_stack_pos_by_child(gp_widget *self, gp_widget *child,
                                         unsigned int *stack_pos);

/**
 * @brief Puts a widget on stack_pos into an overlay.
 *
 * @self An overlay widget.
 * @stack_pos Position of the stack to add to.
 * @child A widget to be put into the stack.
 *
 * @return Returns a widget that previously occupied the position or NULL if it
 *         was empty.
 */
gp_widget *gp_widget_overlay_put(gp_widget *self, unsigned int stack_pos,
                                 gp_widget *child);

#endif /* GP_WIDGET_OVERLAY_H */
