//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <gfxprim.h>

static void *uids;

static gp_pixel fg_rgb;
static gp_pixel bg_rgb;

static void draw(gp_widget *pixmap, gp_event *ev)
{
	gp_event_dump(ev);
	if (ev->type != GP_EV_KEY ||
	    (ev->key.key != GP_BTN_LEFT &&
	     ev->key.key != GP_BTN_PEN))
		return;

	gp_coord x = ev->cursor_x;
	gp_coord y = ev->cursor_y;

	gp_pixmap *p = pixmap->pixmap->pixmap;

	gp_pixel col = gp_rgb_to_pixmap_pixel((fg_rgb >> 16) & 0xff, (fg_rgb >> 8) & 0xff, fg_rgb & 0xff, p);

	gp_putpixel(p, x, y, col);
	gp_circle(p, x, y, 10, col);
	gp_widget_redraw(pixmap);
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

int set_fg_color(gp_widget_event *ev)
{
	gp_widget_event_dump(ev);

	switch (ev->type) {
	case GP_WIDGET_EVENT_NEW:
		ev->self->tbox->filter = GP_TEXT_BOX_FILTER_HEX;
		/* fall through */
	case GP_WIDGET_EVENT_EDIT:
		fg_rgb = strtol(ev->self->tbox->buf, NULL, 16);
		printf("fg_color = 0x%x\n", fg_rgb);
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
		ev->self->tbox->filter = GP_TEXT_BOX_FILTER_HEX;
	/* fall through */
	case GP_WIDGET_EVENT_EDIT:
		bg_rgb = strtol(ev->self->tbox->buf, NULL, 16);
		printf("bg_color = 0x%06x\n", bg_rgb);
	break;
	case GP_WIDGET_EVENT_ACTION:
		pixmap = gp_widget_by_uid(uids, "pixmap", GP_WIDGET_PIXMAP);

		if (pixmap) {
			fill_pixmap(pixmap->pixmap->pixmap);
			gp_widget_redraw(pixmap);
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

	if (ev->type != GP_WIDGET_EVENT_ACTION)
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
