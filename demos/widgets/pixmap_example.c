//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

static gp_htable *uids;

static gp_pixel fg_rgb;
static gp_pixel bg_rgb;

static void draw(gp_widget *pixmap, gp_event *ev)
{
	gp_event_dump(ev);
	if (ev->type != GP_EV_KEY ||
	    (ev->key.key != GP_BTN_LEFT &&
	     ev->key.key != GP_BTN_PEN))
		return;

	gp_coord x = ev->st->cursor_x;
	gp_coord y = ev->st->cursor_y;

	gp_pixmap *p = pixmap->pixmap->pixmap;

	gp_pixel col = gp_rgb_to_pixmap_pixel((fg_rgb >> 16) & 0xff, (fg_rgb >> 8) & 0xff, fg_rgb & 0xff, p);

	gp_putpixel(p, x, y, col);
	gp_circle(p, x, y, 10, col);

	/* Request partiall update */
	gp_widget_pixmap_redraw(pixmap, x - 10, y - 10, 20, 20);
}

static void fill_pixmap(gp_pixmap *p)
{
	gp_pixel col = gp_rgb_to_pixmap_pixel((bg_rgb >> 16) & 0xff, (bg_rgb >> 8) & 0xff, bg_rgb & 0xff, p);
	gp_fill(p, col);
}

static void allocate_backing_pixmap(gp_widget_event *ev)
{
	gp_widget *w = ev->self;

	gp_pixmap_free(w->pixmap->pixmap);

	w->pixmap->pixmap = gp_pixmap_alloc(w->w, w->h, ev->ctx->pixel_type);

	fill_pixmap(w->pixmap->pixmap);
}

int pixmap_on_event(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);

	switch (ev->type) {
	case GP_WIDGET_EVENT_INPUT:
		draw(ev->self, ev->input_ev);
	break;
	case GP_WIDGET_EVENT_RESIZE:
		allocate_backing_pixmap(ev);
	break;
	default:
	break;
	}

	return 0;
}

static void set_color(gp_pixel *col, const char *val, const char *name)
{
	*col = strtol(val, NULL, 16);
	printf("%s = 0x%x\n", name, *col);
}

int set_fg_color(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);

	switch (ev->type) {
	case GP_WIDGET_EVENT_NEW:
		ev->self->tbox->filter = GP_TBOX_FILTER_HEX;
		set_color(&fg_rgb, gp_widget_tbox_text(ev->self), "fg_color");
	break;
	case GP_WIDGET_EVENT_WIDGET:
		if (ev->sub_type != GP_WIDGET_TBOX_EDIT)
			return 0;
		set_color(&fg_rgb, gp_widget_tbox_text(ev->self), "fg_color");
	break;
	default:
	break;
	}

	return 0;
}

int set_bg_color(gp_widget_event *ev)
{
	gp_widget *pixmap;

	gp_widget_event_dump(ev);

	switch (ev->type) {
	case GP_WIDGET_EVENT_NEW:
		ev->self->tbox->filter = GP_TBOX_FILTER_HEX;
		set_color(&bg_rgb, gp_widget_tbox_text(ev->self), "bg_color");
	break;
	case GP_WIDGET_EVENT_WIDGET:
		switch (ev->sub_type) {
		case GP_WIDGET_TBOX_EDIT:
			set_color(&bg_rgb, gp_widget_tbox_text(ev->self), "bg_color");
		break;
		case GP_WIDGET_TBOX_TRIGGER:
			pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

			if (pixmap) {
				fill_pixmap(pixmap->pixmap->pixmap);
				gp_widget_redraw(pixmap);
			}
		break;
		}
	break;
	default:
	break;
	}

	return 0;
}

int button_on_event(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget *pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

	(void)ev;

	if (pixmap) {
		fill_pixmap(pixmap->pixmap->pixmap);
		gp_widget_redraw(pixmap);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_layout_json("pixmap_example.json", &uids);
	if (!layout)
		return 0;

	gp_widget *pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

	gp_widget_event_unmask(pixmap, GP_WIDGET_EVENT_RESIZE);
	gp_widget_event_unmask(pixmap, GP_WIDGET_EVENT_INPUT);

	gp_widgets_main_loop(layout, "Pixmap example", NULL, argc, argv);

	return 0;
}
