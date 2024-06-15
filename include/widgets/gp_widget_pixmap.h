//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_pixmap.h
 * @brief A pixmap widget.
 *
 * Pixmap repaint modes
 * ====================
 *
 * The pixmap widget is allocated as an empty container and there are two
 * different repaint strategies for pixmap widget.
 *
 * Buffered mode
 * -------------
 *
 * In this mode a pixmap of the size of the widget is allocated, by the
 * application, before any drawing is done. The content of the pixmap is
 * preserved i.e. not modified by the library.
 *
 * The allocation is deffered for when the application has been started and the
 * pixel format, we are using for drawing, is known. Only then the library will
 * call the pixmap event handler with a resize event and
 * #gp_widget_render_ctx pointer, which could be used to allocate right
 * backing #gp_pixmap and set the pixmap pointer in the struct
 * gp_widget_pixmap.
 *
 * @note The pixmap has to be resized properly on each resize event as well.
 *
 * @attention The `GP_WIDGET_EVENT_RESIZE` has to be unmasked by
 *            gp_widget_events_unmask() before the application starts.
 *
 * @include{c} demos/widgets/pixmap_example.c
 * @include{json} demos/widgets/pixmap_example.json
 *
 * Unbuffered mode
 * ---------------
 *
 * In this mode the application is passed a temporary buffer in the size of the
 * widget and a #gp_bbox that describes an inner rectangle that has to be
 * repainted. To pass the pointer the `pixmap` member in `struct
 * gp_widget_pixmap` is set temporarily, for the duration of the event handler,
 * and the #gp_bbox is passed down in the #gp_widget_event::bbox.
 *
 * The application is free to ignore the bounding box and repaint the whole
 * pixmap.
 *
 * @note In this mode the content of the pixmap buffer is not preserved between
 *       events so this is mostly useful when pixmap is repainted periodically
 *       and the data is not worth caching.
 *
 * @attention The `GP_WIDGET_EVENT_REDRAW` has to be unmasked by
 *            gp_widget_events_unmask() before the application starts.
 *
 * The `GP_WIDGET_EVENT_RESIZE` can be unmasked as well if you want to be
 * notified when the pixmap is resized, but it's not strictly required in this
 * mode.
 *
 * @include{c} demos/widgets/clock.c
 *
 * Pixmap widget JSON attributes
 * -----------------------------
 *
 * | Attribute |  Type  |   Default   | Description                                                   |
 * |-----------|--------|-------------|---------------------------------------------------------------|
 * |   **w**   |  uint  |             | Minimal pixmap width parsed by gp_widget_size_units_parse().  |
 * |   **h**   |  uint  |             | Minimal pixmap height parsed by gp_widget_size_units_parse(). |
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
 * @brief Create pixmap widget.
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
 * @param min_w Minimal pixmap width
 * @param min_h Minimal pixmap height
 * @param on_event A widget event handler.
 * @param priv A widget event handler private pointer.
 *
 * @return A newly allocated and initialized pixmap widget.
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
 * @param self A pixmap widget.
 * @param x An x offset into the pixmap
 * @param y An y offset into the pixmap
 * @param w A width of a rectangle at offset x,y
 * @param h A heigth of a rectangle at offset x,y
 */
void gp_widget_pixmap_redraw(gp_widget *self,
                             gp_coord x, gp_coord y,
                             gp_size w, gp_size h);

/**
 * @brief Returns pixmap width in pixels.
 *
 * @param self A pixmap widget.
 *
 * @return A pixmap width.
 */
static inline gp_size gp_widget_pixmap_w(gp_widget *self)
{
	return self->w;
}

/**
 * @brief Returns pixmap height in pixels.
 *
 * @param self A pixmap widget.
 *
 * @return A pixmap height.
 */
static inline gp_size gp_widget_pixmap_h(gp_widget *self)
{
	return self->h;
}

/**
 * @brief Marks the whole pixmap to be repainted.
 *
 * @param self A pixmap widget.
 */
void gp_widget_pixmap_redraw_all(gp_widget *self);

#endif /* GP_WIDGET_PIXMAP_H */
