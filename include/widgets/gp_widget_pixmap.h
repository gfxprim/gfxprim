//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_PIXMAP_H
#define GP_WIDGET_PIXMAP_H

struct gp_widget_pixmap {
	unsigned int min_w, min_h;
	/*
	 * If set redraw event is send once on when widget is rendered.
	 */
	int update:1;
	gp_pixmap *pixmap;
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
 * widget to be repainted. You will get the redraw event if update flag has been
 * set prior to widget redraw.
 *
 * Or you can leave the pixmap pointer to be NULL in which case the library
 * will fill it just for the duration of redraw event.
 *
 * @min_w Minimal pixmap width
 * @min_h Minimal pixmap height
 *
 * @return A pixmap widget
 */
gp_widget *gp_widget_pixmap_new(unsigned int min_w, unsigned int min_h,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv);

/**
 * @brief Sets a bitmap widget to unbuffered mode.
 *
 * When called GP_WIDGET_EVENT_REDRAW is send before pixmap is blit on the
 * screen for the buffered case.
 *
 * @self A pixmap widget.
 */
static inline void gp_widget_pixmap_update(gp_widget *self)
{
	self->pixmap->update = 1;
}

#endif /* GP_WIDGET_PIXMAP_H */
