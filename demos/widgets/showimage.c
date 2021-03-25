//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2007-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>
#include <loaders/gp_loaders.h>

static gp_pixmap *image;

static void redraw(gp_widget_event *ev)
{
	gp_widget *pixmap = ev->self;
	gp_pixmap *dst = pixmap->pixmap->pixmap;
	gp_offset *offset = ev->ptr;

	if (image)
		gp_blit_xywh(image, offset->x, offset->y, dst->w, dst->h, dst, 0, 0);
	else
		gp_fill(dst, ev->ctx->fg_color);
}

static void alloc_and_blit(gp_widget_event *ev)
{
	gp_widget *w = ev->self;

	gp_pixmap *p = gp_pixmap_alloc(w->w, w->h, ev->ctx->pixel_type);

	if (image)
		gp_blit_xywh(image, 0, 0, p->w, p->h, p, 0, 0);

	gp_pixmap_free(w->pixmap->pixmap);
	w->pixmap->pixmap = p;
}

int pixmap_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_REDRAW:
		redraw(ev);
		return 1;
	case GP_WIDGET_EVENT_RESIZE:
		alloc_and_blit(ev);
		return 1;
	default:
		return 0;
	}
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_grid_new(1, 1, 0);
	gp_widget *area = gp_widget_scroll_area_new(320, 240, NULL);
	gp_size w = 320, h = 240;

	layout->align = GP_FILL;
	area->align = GP_FILL;

	gp_widgets_getopt(&argc, &argv);

	gp_widget_grid_put(layout, 0, 0, area);

	if (argv[0]) {
		image = gp_load_image(argv[0], NULL);
		if (image) {
			w = image->w;
			h = image->h;
		}
	}

	gp_widget *pixmap = gp_widget_pixmap_new(w, h, pixmap_on_event, NULL);

	gp_widget_event_unmask(pixmap, GP_WIDGET_EVENT_REDRAW);

	gp_widget_event_unmask(pixmap, GP_WIDGET_EVENT_RESIZE);

	gp_widget_scroll_area_put(area, pixmap);

	gp_widgets_main_loop(layout, "Show Image", NULL, 0, NULL);

	return 0;
}
