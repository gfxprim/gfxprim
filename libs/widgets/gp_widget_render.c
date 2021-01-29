//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_fds.h>
#include <backends/gp_backends.h>
#include <input/gp_input_driver_linux.h>
#include <gp_widget_render.h>
#include <gp_widget_ops.h>
#include <gp_key_repeat_timer.h>
#include <widgets/gp_widget_dialog.h>
#include <widgets/gp_widget_app.h>

static struct gp_text_style font = {
	.pixel_xmul = 1,
	.pixel_ymul = 1,
	.font = &gp_default_font,
};

static struct gp_text_style font_bold = {
	.pixel_xmul = 2,
	.pixel_ymul = 2,
	.pixel_xspace = -1,
	.pixel_yspace = -1,
	.font = &gp_default_font,
};

static struct gp_text_style font_big = {
	.pixel_xmul = 2,
	.pixel_ymul = 2,
	.font = &gp_default_font,
};

static struct gp_text_style font_big_bold = {
	.pixel_xmul = 3,
	.pixel_ymul = 3,
	.pixel_xspace = -1,
	.pixel_yspace = -1,
	.font = &gp_default_font,
};

static struct gp_widget_render_ctx ctx = {
	.text_color = 0,
	.font = &font,
	.font_bold = &font_bold,
	.font_big = &font_big,
	.font_big_bold = &font_big_bold,
	.font_mono = &font,
	.padd = 4,
	.dclick_ms = 500,
};

static gp_pixel fill_color;

static int font_size = 16;
static gp_font_face *render_font;
static gp_font_face *render_font_bold;
static gp_font_face *render_font_big;
static gp_font_face *render_font_big_bold;
static gp_font_face *render_font_mono;

static const char *arg_fonts;
static const char *input_str;

static void set_big_font(int mul)
{
	font_big.pixel_xmul = mul;
	font_big.pixel_ymul = mul;

	font_bold.pixel_xmul = 1;
	font_bold.pixel_ymul = 1;
	font_bold.pixel_xspace = 0;
	font_bold.pixel_yspace = 0;

	font_big_bold.pixel_xmul = mul;
	font_big_bold.pixel_ymul = mul;
	font_big_bold.pixel_xspace = 0;
	font_big_bold.pixel_yspace = 0;
}

static void set_default_font(void)
{
	/* Embolding */
	font_bold.pixel_xmul = 2;
	font_bold.pixel_ymul = 2;
	font_bold.pixel_xspace = -1;
	font_bold.pixel_yspace = -1;

	font_big.pixel_xmul = 2;
	font_big.pixel_ymul = 2;

	font_big_bold.pixel_xmul = 3;
	font_big_bold.pixel_ymul = 3;
	font_big_bold.pixel_xspace = -1;
	font_big_bold.pixel_yspace = -1;
}

static void init_fonts(void)
{
	if (arg_fonts) {
		if (!strcmp(arg_fonts, "default")) {
			set_default_font();
			return;
		}

		if (!strcmp(arg_fonts, "haxor-15")) {
			font.font = gp_font_haxor_narrow_15;
			font_bold.font = gp_font_haxor_narrow_bold_15;
			font_big.font = gp_font_haxor_narrow_15;
			font_big_bold.font = gp_font_haxor_narrow_bold_15;
			set_big_font(2);
			return;
		}

		if (!strcmp(arg_fonts, "haxor-16")) {
			font.font = gp_font_haxor_narrow_16;
			font_bold.font = gp_font_haxor_narrow_bold_16;
			font_big.font = gp_font_haxor_narrow_16;
			font_big_bold.font = gp_font_haxor_narrow_bold_16;
			set_big_font(2);
			return;
		}

		if (!strcmp(arg_fonts, "haxor-17")) {
			font.font = gp_font_haxor_narrow_17;
			font_bold.font = gp_font_haxor_narrow_bold_17;
			font_big.font = gp_font_haxor_narrow_17;
			font_big_bold.font = gp_font_haxor_narrow_bold_17;
			set_big_font(2);
			return;
		}

		int size = atoi(arg_fonts);

		if (size < 0 || size > 200) {
			GP_WARN("Inavlid font '%s'!", arg_fonts);
			return;
		}

		font_size = size;
	}

	gp_font_face *ffont = gp_font_face_fc_load("DroidSans", 0, font_size);
	gp_font_face *ffont_bold = gp_font_face_fc_load("DroidSans:Bold", 0, font_size);
	gp_font_face *ffont_big = gp_font_face_fc_load("DroidSans", 0, 1.8 * font_size);
	gp_font_face *ffont_big_bold = gp_font_face_fc_load("DroidSans:Bold", 0, 1.8 * font_size);
	gp_font_face *ffont_mono = gp_font_face_fc_load("DroidSansMono", 0, font_size);

	if (!ffont || !ffont_bold || !ffont_big || !ffont_big_bold || !ffont_mono) {
		gp_font_face_free(ffont);
		gp_font_face_free(ffont_bold);
		gp_font_face_free(ffont_big);
		gp_font_face_free(ffont_big_bold);
		gp_font_face_free(ffont_mono);
		set_default_font();
		return;
	}

	gp_font_face_free(render_font);
	gp_font_face_free(render_font_bold);
	gp_font_face_free(render_font_big);
	gp_font_face_free(render_font_big_bold);
	gp_font_face_free(render_font_mono);

	render_font = ffont;
	render_font_bold = ffont_bold;
	render_font_big = ffont_big;
	render_font_big_bold = ffont_big_bold;
	render_font_mono = ffont_mono;

	ctx.font->font = ffont;
	ctx.font_bold->font = ffont_bold;
	ctx.font_big->font = ffont_big;
	ctx.font_big_bold->font = ffont_big_bold;
	ctx.font_mono->font = ffont_mono;
	set_big_font(1);
}

void gp_widget_render_init(void)
{
	init_fonts();

	ctx.padd = 2 * gp_text_descent(ctx.font);
}

static gp_backend *backend;
static gp_widget *app_layout;

void gp_widget_render_zoom(int zoom_inc)
{
	if (font_size + zoom_inc < 5)
		return;

	font_size += zoom_inc;

	gp_widget_render_init();
	gp_widget_resize(app_layout);
	gp_widget_redraw(app_layout);
}

static void timer_event(gp_event *ev)
{
	struct gp_widget *widget = ev->tmr->priv;

	gp_widget_ops_event(widget, &ctx, ev);

	ev->tmr->priv = NULL;
}

static struct gp_timer timers[10];

void gp_widget_render_timer(gp_widget *self, int flags, unsigned int timeout_ms)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(timers); i++) {
		if (timers[i].priv == self) {
			if (flags & GP_TIMER_RESCHEDULE) {
				gp_backend_rem_timer(backend, &timers[i]);
				timers[i].expires = timeout_ms;
				gp_backend_add_timer(backend, &timers[i]);
				return;
			}

			GP_WARN("Timer for widget %p (%s) allready running!",
			        self, gp_widget_type_id(self));
			return;
		}

		if (!timers[i].priv)
			break;
	}

	if (i >= GP_ARRAY_SIZE(timers)) {
		GP_WARN("All %zu timers used!", GP_ARRAY_SIZE(timers));
		gp_timer_queue_dump(backend->timers);
	}

	timers[i].expires = timeout_ms;
	timers[i].id = gp_widget_type_id(self);
	timers[i].priv = self;

	gp_backend_add_timer(backend, &timers[i]);
}

void gp_widget_render_timer_cancel(gp_widget *self)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(timers); i++) {
		if (timers[i].priv == self) {
			gp_backend_rem_timer(backend, &timers[i]);
			timers[i].priv = NULL;
			return;
		}

		if (!timers[i].priv)
			break;
	}
}

static int in_dialog;

void gp_widgets_redraw(struct gp_widget *layout)
{
	if (!layout) {
		GP_DEBUG(1, "Redraw called with NULL layout!");
		return;
	}

	if (!layout->redraw && !layout->redraw_child)
		return;

	if (gp_pixmap_w(backend->pixmap) < layout->w ||
	    gp_pixmap_h(backend->pixmap) < layout->h) {
		gp_backend_resize(backend, layout->w, layout->h);
	}

	if (gp_pixmap_w(backend->pixmap) == 0 ||
	    gp_pixmap_h(backend->pixmap) == 0)
		return;

	gp_bbox flip = {};

	ctx.flip = &flip;
	gp_widget_render(layout, &ctx, 0);
	ctx.flip = NULL;

	if (in_dialog)
		gp_rect_xywh(ctx.buf, layout->x, layout->y, layout->w, layout->h, ctx.text_color);

	if (gp_bbox_empty(flip))
		return;

	GP_DEBUG(1, "Updating area " GP_BBOX_FMT, GP_BBOX_PARS(flip));

	gp_backend_update_rect_xywh(backend, flip.x, flip.y, flip.w, flip.h);
}

static char *backend_init_str = "x11";

void gp_widget_timer_queue_switch(gp_timer **);

void gp_widgets_layout_init(gp_widget *layout, const char *win_tittle)
{
	gp_widget_render_init();

	backend = gp_backend_init(backend_init_str, win_tittle);
	if (!backend)
		exit(1);

	gp_widget_timer_queue_switch(&backend->timers);

	gp_key_repeat_timer_init(&backend->event_queue, &backend->timers);

	ctx.buf = backend->pixmap;
	ctx.pixel_type = backend->pixmap->pixel_type;

	ctx.bg_color = gp_rgb_to_pixmap_pixel(0xdd, 0xdd, 0xdd, ctx.buf);
	ctx.fg_color = gp_rgb_to_pixmap_pixel(0xee, 0xee, 0xee, ctx.buf);
	ctx.fg2_color = gp_rgb_to_pixmap_pixel(0x77, 0xbb, 0xff, ctx.buf);
	ctx.sel_color = gp_rgb_to_pixmap_pixel(0x11, 0x99, 0xff, ctx.buf);
	ctx.alert_color = gp_rgb_to_pixmap_pixel(0xff, 0x55, 0x55, ctx.buf);
	ctx.accept_color = gp_rgb_to_pixmap_pixel(0x00, 0xbb, 0x00, ctx.buf);
	fill_color = gp_rgb_to_pixmap_pixel(0x44, 0x44, 0x44, ctx.buf);

	gp_widget_calc_size(layout, &ctx, 0, 0, 1);

	gp_backend_resize(backend, layout->w, layout->h);

	app_layout = layout;

	int flag = 0;

	if (layout->w != gp_pixmap_w(backend->pixmap) ||
	    layout->h != gp_pixmap_h(backend->pixmap)) {
		gp_fill(backend->pixmap, fill_color);
		flag = 1;
	}

	if (gp_pixmap_w(backend->pixmap) == 0 ||
	    gp_pixmap_h(backend->pixmap) == 0)
		return;

	gp_widget_render(layout, &ctx, flag);
	gp_backend_flip(backend);
}

const gp_widget_render_ctx *gp_widgets_render_ctx(void)
{
	return &ctx;
}

static int (*app_event_callback)(gp_event *);

void gp_widgets_register_callback(int (*event_callback)(gp_event *))
{
	app_event_callback = event_callback;
}

int gp_widgets_event(gp_event *ev, gp_widget *layout)
{
	int handled = 0;

	gp_handle_key_repeat_timer(ev);

	switch (ev->type) {
	case GP_EV_KEY:
		if (gp_event_get_key(ev, GP_KEY_LEFT_CTRL) &&
		    ev->code == GP_EV_KEY_DOWN) {
			switch (ev->val) {
			case GP_KEY_UP:
				gp_widget_render_zoom(1);
				handled = 1;
			break;
			case GP_KEY_DOWN:
				gp_widget_render_zoom(-1);
				handled = 1;
			break;
			}
		}
		if ((gp_event_get_key(ev, GP_KEY_LEFT_ALT) ||
		     gp_event_get_key(ev, GP_KEY_LEFT_ALT)) &&
		     ev->code == GP_EV_KEY_DOWN) {
			switch (ev->val) {
			case GP_KEY_F4:
				return 1;
			}
		}
	break;
	case GP_EV_SYS:
		switch (ev->code) {
		case GP_EV_SYS_RESIZE:
			gp_backend_resize_ack(backend);
			ctx.buf = backend->pixmap;
			gp_fill(backend->pixmap, fill_color);
			gp_widget_render(layout, &ctx, 1);
			gp_backend_flip(backend);
			handled = 1;
		break;
		case GP_EV_SYS_QUIT:
			return 1;
		break;
		}
	break;
	case GP_EV_TMR:
		timer_event(ev);
		handled = 1;
	break;
	}

	if (handled)
		return 0;

	handled = gp_widget_input_event(layout, &ctx, ev);

	if (handled)
		return 0;

	if (app_event_callback)
		app_event_callback(ev);

	return 0;
}

int gp_widgets_process_events(gp_widget *layout)
{
	gp_event ev;

	while (gp_backend_poll_event(backend, &ev)) {
		//gp_event_dump(&ev);
		//fflush(stdout);
		if (gp_widgets_event(&ev, layout))
			gp_widgets_exit(0);
	}

	return 0;
}

static void print_options(int exit_val)
{
	printf("Options:\n--------\n");
	printf("\t-b backend init string (pass -b help for options)\n");
	printf("\t-f fonts\n\t\tdefault\n\t\thaxor-15\n\t\thaxor-16\n\t\thaxor-17\n");
	printf("\t-f uint\n\t\tsets the true-type font size\n");
	printf("\t-i input_string\n");
	exit(exit_val);
}

void gp_widgets_getopt(int *argc, char **argv[])
{
	int opt;

	while ((opt = getopt(*argc, *argv, "b:f:hi:")) != -1) {
		switch (opt) {
		case 'b':
			backend_init_str = optarg;
		break;
		case 'h':
			print_options(0);
		break;
		case 'f':
			arg_fonts = optarg;
		break;
		case 'i':
			input_str = optarg;
		break;
		default:
			print_options(1);
		}
	}

	*argv = *argv + optind;
	*argc -= optind;
}

static gp_widget *win_layout;

gp_widget *gp_widget_layout_replace(gp_widget *layout)
{
	gp_widget *ret = win_layout;

	gp_widget_calc_size(layout, &ctx, 0, 0, 1);

	if (gp_pixmap_w(backend->pixmap) < layout->w ||
	    gp_pixmap_h(backend->pixmap) < layout->h) {
		gp_backend_resize(backend, layout->w, layout->h);
	}

	gp_widget_resize(layout);
	gp_widget_redraw(layout);

	win_layout = layout;

	return ret;
}

static int backend_event(struct gp_fd *self, struct pollfd *pfd)
{
	(void) pfd;
	(void) self;

	if (gp_widgets_process_events(win_layout))
		return 1;

	return 0;
}

static int input_event(struct gp_fd *self, struct pollfd *pfd)
{
	while (gp_input_linux_read(self->priv, &backend->event_queue) > 0);

	if (gp_widgets_process_events(win_layout))
		return 1;

	return 0;
}

static struct gp_fds fds = GP_FDS_INIT;

struct gp_fds *gp_widgets_fds = &fds;

int gp_widget_dialog_run(gp_widget_dialog *dialog)
{
	gp_widget *saved = gp_widget_layout_replace(dialog->layout);

	dialog->dialog_exit = 0;
	in_dialog = 1;

	for (;;) {
		int ret = gp_fds_poll(&fds, gp_backend_timer_timeout(backend));
		/* handle timers */
		if (ret == 0) {
			gp_backend_poll(backend);
			gp_widgets_process_events(dialog->layout);
		}

		if (dialog->dialog_exit) {
			//TODO: this will flicker
			gp_fill(backend->pixmap, fill_color);
			gp_backend_flip(backend);

			in_dialog = 0;

			gp_widget_layout_replace(saved);

			return dialog->dialog_exit;
		}

		gp_widgets_redraw(dialog->layout);
	}
}

void gp_widgets_main_loop(gp_widget *layout, const char *label,
                          void (*init)(void), int argc, char *argv[])
{
	if (argv)
		gp_widgets_getopt(&argc, &argv);

	gp_widgets_layout_init(layout, label);

	win_layout = layout;

	if (init)
		init();

	gp_fds_add(&fds, backend->fd, POLLIN, backend_event, NULL);

	if (input_str) {
		gp_input_linux *input = gp_input_linux_by_devstr(input_str);

		if (input)
			gp_fds_add(&fds, input->fd, POLLIN, input_event, input);
	}

	for (;;) {
		int ret = gp_fds_poll(&fds, gp_backend_timer_timeout(backend));
		/* handle timers */
		if (ret == 0) {
			gp_backend_poll(backend);
			gp_widgets_process_events(win_layout);
		}
		gp_widgets_redraw(win_layout);
	}
}

void gp_widgets_exit(int exit_value)
{
	gp_app_send_event(GP_WIDGET_EVENT_FREE);

	gp_widget_free(win_layout);
	gp_backend_exit(backend);
	gp_font_face_free(render_font);
	gp_font_face_free(render_font_bold);
	gp_font_face_free(render_font_big);
	gp_font_face_free(render_font_big_bold);
	gp_fds_clear(gp_widgets_fds);

	exit(exit_value);
}
