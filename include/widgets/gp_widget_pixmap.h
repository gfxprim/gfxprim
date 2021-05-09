//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_PIXMAP_H
#define GP_WIDGET_PIXMAP_H

#include <widgets/gp_widget_size_units.h>

struct gp_widget_pixmap {
	gp_widget_size min_w;
	gp_widget_size min_h;
	gp_pixmap *pixmap;
	int bbox_set:1;
	int redraw_all:1;
	/** Bounding box */
	gp_bbox bbox;
};

/**
 * @brief Allocate and initialize a pixmap widget.
 *
 * If pixmap widget has align set to fill the widget will grow into available
 * space, otherwise it will be always precisely min_w x min_h in size.
 *
 * There are two modes of operation:
 *
 * Either you allocate a backing pixmap in the resize event. Then you draw into
 * it and the library will simply copy the buffer when application requests the
 * widget to be repainted.
 *
 * Or you can leave the pixmap pointer to be NULL in which case the library
 * will fill it just for the duration of redraw event.
 *
 * @min_w Minimal pixmap width
 * @min_h Minimal pixmap height
 *
 * @return A pixmap widget
 */
gp_widget *gp_widget_pixmap_new(gp_widget_size min_w, gp_widget_size min_h,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv);

/**
 * @brief Requests partial update of the image.
 *
 * This function is useful in the case that the application renders into
 * pre-allocated pixmap and only the part of the buffer should be repainted. If
 * called more than once before pixmap update on the screen the areas are merged
 * into one that contains both bounding boxes.
 *
 * @self A pixmap widget.
 * @x An x offset into the pixmap
 * @y An y offset into the pixmap
 * @w A width of a rectangle at offset x,y
 * @h A heigth of a rectangle at offset x,y
 */
void gp_widget_pixmap_redraw(gp_widget *self,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h);

/**
 * @brief Marks the whole pixmap to be repainted.
 *
 * @self A pixmap widget.
 */
void gp_widget_pixmap_redraw_all(gp_widget *self);

#endif /* GP_WIDGET_PIXMAP_H */
