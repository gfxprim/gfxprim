//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_hvbox.h
 * @brief A horizontal and vertical box widget.
 *
 * Hbox and vbox widgets are one-dimensional gp_widget_grid_new() widgets
 * with rows=1, cols=1 respectively and with a simplified JSON attribute list.
 *
 * Hbox example buttons apart
 * --------------------------
 *
 * @include{json} demos/widgets/test_layouts/hbox_buttons_apart.json
 * @image html widgets_buttons_apart.png
 *
 * The hbox is uniform, that means that both buttons have the same width,
 * otherwise the button OK would be smaller as the button label is shorter.
 *
 * The padd between the buttons is the only element that can grow, so it fills
 * awailable space as the hbox grows.
 *
 * Hbox example buttons right
 * --------------------------
 *
 * @include{json} demos/widgets/test_layouts/hbox_buttons_right.json
 *
 * @image html widgets_buttons_right.png
 *
 * The hbox is uniform, that means that both buttons have the same width,
 * otherwise the button OK would be smaller as the button label is shorter.
 *
 * The padd on the left is the only element that can grow, so it fills the
 * available space as the hbox grows.
 *
 * Hbox example buttons fill
 * -------------------------
 *
 * @include{json} demos/widgets/test_layouts/hbox_buttons_fill.json
 *
 * @image html widgets_buttons_fill.png
 *
 * The hbox is uniform, that means that both buttons have the same width,
 * otherwise the button OK would be smaller as the button label is shorter.
 *
 * Both buttons have fill coeficient 4 and the space between them has fill
 * coeficient 1. Available space is divided by 4:1:4.
 *
 * Hbox and Vbox JSON attributes
 * -----------------------------
 *
 * |  Attribute    |  Type  |     Default     | Description                   |
 * |---------------|--------|-----------------|-------------------------------|
 * |  **border**   | string |    all **1**    | See gp_widget_grid.h          |
 * | **cell-fill** | string |  cnt x **1**    | Cell fill coeficients.        |
 * | **padd-fill** | string | (cnt+1) x **0** | Padding fill coeficients.     |
 * |   **padd**    | string | (cnt+1) x **1** | Padding sizes between cells.  |
 * |  **frame**    |  bool  |      false      | Draws a frame around the box. |
 * | **uniform**   |  bool  |      false      | All cells have the same size. |
 * | **widgets**   |  array |                 | Array of widgets.             |
 */
#ifndef GP_WIDGET_HVBOX_H
#define GP_WIDGET_HVBOX_H

#include <widgets/gp_widget_grid.h>

/**
 * @brief Creates a hbox i.e. grid with single row.
 *
 * @param cols A number of columns in the box.
 * @param flags A bitwise combination of flags.
 *
 * @return A newly allocated and initialized hbox widget.
 */
static inline gp_widget *gp_widget_hbox_new(unsigned int cols,
                                            enum gp_widget_grid_flags flags)
{
	return gp_widget_grid_new(cols, 1, flags);
}

/**
 * @brief Creates a vbox i.e. grid with single column.
 *
 * @param rows A number of columns in the box.
 * @param flags A bitwise combination of flags.
 *
 * @return A newly allocated and initialized vbox widget.
 */
static inline gp_widget *gp_widget_vbox_new(unsigned int rows,
                                            enum gp_widget_grid_flags flags)
{
	return gp_widget_grid_new(1, rows, flags);
}

/**
 * @brief Puts a widget into a hbox or vbox.
 *
 * @param self A hbox or vbox widget.
 * @param pos A position to insert the child into, column for hbox and row for vbox.
 * @param child A child widget to insert.
 *
 * @return Previous child occupying the slot or NULL if it was empty.
 */
gp_widget *gp_widget_box_put(gp_widget *self, unsigned int pos, gp_widget *child);

/**
 * @brief Removes a widget from hbox or vbox.
 *
 * @param self A hbox or vbox widget.
 * @param pos A position to remove the child from.
 *
 * @return A child widget removed from the slot or NULL if it was empty.
 */
gp_widget *gp_widget_box_rem(gp_widget *self, unsigned int pos);

#endif /* GP_WIDGET_VHBOX_H */
