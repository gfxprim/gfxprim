//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2007-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <math.h>
#include <time.h>
#include <widgets/gp_widgets.h>

static void draw(gp_widget *pixmap, const gp_widget_render_ctx *ctx, gp_size line_w)
{
	gp_pixmap *pix = gp_widget_pixmap_get(pixmap);
	gp_pixel red = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, pix);
	gp_pixel fg = ctx->text_color;

	gp_fill(pix, ctx->bg_color);

	int w = pix->w;
	int h = pix->h;
	int r = GP_MIN(w, h)/2 - 1;
	int cx = r + (w - 2 * r)/2;
	int cy = r + (h - 2 * r)/2;

	gp_fill_circle(pix, cx, cy, r, ctx->fg_color);
	gp_fill_ring(pix, cx, cy, r, r-2*line_w, fg);

	gp_line_th(pix, cx, cy - r, cx, cy - r + 8 * line_w, line_w, fg);
	gp_line_th(pix, cx, cy + r, cx, cy + r - 8 * line_w, line_w, fg);
	gp_line_th(pix, cx - r, cy, cx - r + 8 * line_w, cy, line_w, fg);
	gp_line_th(pix, cx + r, cy, cx + r - 8 * line_w, cy, line_w, fg);

	int csa = r * sin(M_PI/6);
	int cca = r * cos(M_PI/6);
	int csb = (r - 4 * line_w) * sin(M_PI/6);
	int ccb = (r - 4 * line_w) * cos(M_PI/6);

	gp_line_th(pix, cx - csa, cy - cca, cx - csb, cy - ccb, line_w, fg);
	gp_line_th(pix, cx + csa, cy - cca, cx + csb, cy - ccb, line_w, fg);
	gp_line_th(pix, cx - csa, cy + cca, cx - csb, cy + ccb, line_w, fg);
	gp_line_th(pix, cx + csa, cy + cca, cx + csb, cy + ccb, line_w, fg);

	gp_line_th(pix, cx - cca, cy - csa, cx - ccb, cy - csb, line_w, fg);
	gp_line_th(pix, cx + cca, cy - csa, cx + ccb, cy - csb, line_w, fg);
	gp_line_th(pix, cx - cca, cy + csa, cx - ccb, cy + csb, line_w, fg);
	gp_line_th(pix, cx + cca, cy + csa, cx + ccb, cy + csb, line_w, fg);

	time_t timestamp;
	struct tm *tm;

	time(&timestamp);
	tm = localtime(&timestamp);

	float angle;

	angle = 2 * M_PI * (tm->tm_hour%12) / 12 - M_PI/2;

	gp_line_th(pix, cx, cy,
	           cx + 0.6 * r * cos(angle), cy + 0.6 * r * sin(angle), line_w, fg);

	angle = 2 * M_PI * tm->tm_min / 60 - M_PI/2;

	gp_line_th(pix, cx, cy,
	           cx + 0.8 * r * cos(angle), cy + 0.8 * r * sin(angle), line_w, fg);

	angle = 2 * M_PI * tm->tm_sec / 60 - M_PI/2;

	gp_line_th(pix, cx, cy,
	           cx + 0.9 * r * cos(angle), cy + 0.8 * r * sin(angle), line_w, red);
}

int pixmap_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_REDRAW:
		draw(ev->self, ev->ctx, ev->ctx->fr_thick);
		printf("BBOX " GP_BBOX_FMT "\n", GP_BBOX_PARS((*ev->bbox)));
		return 1;
	default:
		return 0;
	}
}

static uint32_t timer_callback(gp_timer *self)
{
	gp_widget *pixmap = self->priv;
	gp_widget_pixmap_redraw_all(pixmap);
	return 1000;
}

static gp_timer tmr = {
	.expires = 1000,
	.callback = timer_callback,
	.id = "Clock redraw"
};

gp_app_info app_info = {
	.name = "Analog Clock",
	.desc = "Simple analog clock",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://gfxprim.ucw.cz",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2007-2023"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_widget_grid_new(1, 1, 0);
	gp_widget *pixmap = gp_widget_pixmap_new(GP_WIDGET_SIZE(100, 0, 0),
	                                         GP_WIDGET_SIZE(100, 0, 0),
	                                         pixmap_on_event, NULL);

	gp_widget_grid_put(layout, 0, 0, pixmap);

	layout->align = GP_FILL;
	pixmap->align = GP_FILL;

	tmr.priv = pixmap;

	gp_widget_events_unmask(pixmap, GP_WIDGET_EVENT_REDRAW);

	gp_widgets_timer_ins(&tmr);

	gp_widgets_main_loop(layout, NULL, argc, argv);
}
