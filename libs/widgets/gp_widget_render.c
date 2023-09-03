//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_poll.h>
#include <utils/gp_user_path.h>
#include <backends/gp_backends.h>

#include <widgets/gp_widget_render.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_key_repeat_timer.h>
#include <widgets/gp_dialog.h>
#include <widgets/gp_widget_app.h>
#include <widgets/gp_widgets_task.h>
#include <widgets/gp_widget_keys.h>
#include <widgets/gp_app_info.h>
#include <widgets/gp_widget_poll.h>

#include "gp_widgets_internal.h"

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

static struct gp_text_style font_mono = {
	.pixel_xmul = 1,
	.pixel_ymul = 1,
	.font = &gp_default_font,
};

static struct gp_text_style font_mono_bold = {
	.pixel_xmul = 2,
	.pixel_ymul = 2,
	.pixel_xspace = -1,
	.pixel_yspace = -1,
};

struct gp_widget_render_ctx __attribute__((visibility ("hidden"))) ctx = {
	.text_color = 0,
	.font = &font,
	.font_bold = &font_bold,
	.font_big = &font_big,
	.font_big_bold = &font_big_bold,
	.font_mono = &font_mono,
	.font_mono_bold = &font_mono_bold,
	.padd = 10,
	.fr_thick = 1,
	.cur_thick = 1,
	.fr_round = 3,
	.dclick_ms = 500,
	.font_size = 16,
	.color_scheme = GP_WIDGET_COLOR_SCHEME_DEFAULT,
};

static gp_font_face *render_font;
static gp_font_face *render_font_bold;
static gp_font_face *render_font_big;
static gp_font_face *render_font_big_bold;
static gp_font_face *render_font_mono;
static gp_font_face *render_font_mono_bold;

static const char *str_font_size;
static const char *font_family;
static const char *input_str;

static void update_ctx_sizes(void)
{
	ctx.padd = 2 * gp_text_descent(ctx.font);

	ctx.fr_thick = ctx.font_size / 25 + 1;
	ctx.fr_round = ctx.font_size / 15 + 3;
	ctx.cur_thick = ctx.font_size / 25 + 1;
}

static int try_compiled_in_font_family(const char *family_name, unsigned int mul)
{
	const gp_font_family *family;
	const gp_font_face *regular, *regular_bold, *mono, *mono_bold;

	family = gp_font_family_lookup(family_name);
	if (!family)
		return 0;

	regular = gp_font_family_face_lookup(family, GP_FONT_REGULAR);
	regular_bold = gp_font_family_face_lookup(family, GP_FONT_REGULAR | GP_FONT_BOLD);
	mono = gp_font_family_face_lookup(family, GP_FONT_MONO);
	mono_bold = gp_font_family_face_lookup(family, GP_FONT_MONO | GP_FONT_BOLD);

	/* Fallback to monospace if regular is not present */
	if (!regular) {
		regular = mono;
		regular_bold = mono_bold;
	}

	if (!regular || !mono)
		return 0;

	gp_text_style_normal(&font, regular, mul);
	gp_text_style_normal(&font_mono, mono, mul);
	gp_text_style_normal(&font_big, regular, 2 * mul);

	if (!regular_bold) {
		gp_text_style_embold(&font_bold, regular, mul);
		gp_text_style_embold(&font_big_bold, regular, 2 * mul);
	} else {
		gp_text_style_normal(&font_bold, regular_bold, mul);
		gp_text_style_normal(&font_big_bold, regular_bold, 2 * mul);
	}

	if (!mono_bold)
		gp_text_style_embold(&font_mono_bold, mono, mul);
	else
		gp_text_style_normal(&font_mono_bold, mono_bold, mul);

	update_ctx_sizes();

	return 1;
}


static void init_fonts(void)
{
	int size = 1;

	if (str_font_size) {
		size = atoi(str_font_size);

		if (size <= 0 || size > 100) {
			GP_WARN("Inavlid font size '%s'!", str_font_size);
			return;
		}

		ctx.font_size = size;
	}

	if (font_family) {
		if (!try_compiled_in_font_family(font_family, size))
			GP_WARN("Failed to load compiled in font '%s'", font_family);
		return;
	}

	gp_font_face *ffont = gp_font_face_fc_load("DroidSans", 0, ctx.font_size);
	gp_font_face *ffont_bold = gp_font_face_fc_load("DroidSans:Bold", 0, ctx.font_size);
	gp_font_face *ffont_big = gp_font_face_fc_load("DroidSans", 0, 1.8 * ctx.font_size);
	gp_font_face *ffont_big_bold = gp_font_face_fc_load("DroidSans:Bold", 0, 1.8 * ctx.font_size);
	gp_font_face *ffont_mono = gp_font_face_fc_load("Monospace", 0, ctx.font_size);
	gp_font_face *ffont_mono_bold = gp_font_face_fc_load("Monospace:Bold", 0, ctx.font_size);

	if (!ffont || !ffont_bold || !ffont_big || !ffont_big_bold || !ffont_mono || !ffont_mono_bold) {
		gp_font_face_free(ffont);
		gp_font_face_free(ffont_bold);
		gp_font_face_free(ffont_big);
		gp_font_face_free(ffont_big_bold);
		gp_font_face_free(ffont_mono);
		gp_font_face_free(ffont_mono_bold);
		try_compiled_in_font_family("gfxprim", size);
		return;
	}

	gp_font_face_free(render_font);
	gp_font_face_free(render_font_bold);
	gp_font_face_free(render_font_big);
	gp_font_face_free(render_font_big_bold);
	gp_font_face_free(render_font_mono);
	gp_font_face_free(render_font_mono_bold);

	render_font = ffont;
	render_font_bold = ffont_bold;
	render_font_big = ffont_big;
	render_font_big_bold = ffont_big_bold;
	render_font_mono = ffont_mono;
	render_font_mono_bold = ffont_mono_bold;

	gp_text_style_normal(ctx.font, ffont, 1);
	gp_text_style_normal(ctx.font_bold, ffont_bold, 1);
	gp_text_style_normal(ctx.font_big, ffont_big, 1);
	gp_text_style_normal(ctx.font_big_bold, ffont_big_bold, 1);
	gp_text_style_normal(ctx.font_mono, ffont_mono, 1);
	gp_text_style_normal(ctx.font_mono_bold, ffont_mono_bold, 1);

	update_ctx_sizes();
}

struct gp_json_struct render_ctx_desc[] = {
	GP_JSON_SERDES_UINT8(struct gp_widget_render_ctx, cur_thick, GP_JSON_SERDES_OPTIONAL, 1, 255, "cursor_thickness"),
	GP_JSON_SERDES_UINT16(struct gp_widget_render_ctx, dclick_ms, GP_JSON_SERDES_OPTIONAL, 100, 10000, "double_click_ms"),
	GP_JSON_SERDES_UINT8(struct gp_widget_render_ctx, font_size, GP_JSON_SERDES_OPTIONAL, 10, 100, "font_size"),
	GP_JSON_SERDES_UINT8(struct gp_widget_render_ctx, fr_round, GP_JSON_SERDES_OPTIONAL, 0, 255, "frame_roundness"),
	GP_JSON_SERDES_UINT8(struct gp_widget_render_ctx, fr_thick, GP_JSON_SERDES_OPTIONAL, 1, 255, "frame_thickness"),
	GP_JSON_SERDES_UINT8(struct gp_widget_render_ctx, padd, GP_JSON_SERDES_OPTIONAL, 2, 255, "padding"),
	{}
};

static void render_ctx_init(void)
{
	char *path = gp_user_path(".config", "gfxprim.json");

	char buf[1024];
	gp_json_val val = {.buf = buf, .buf_size = sizeof(buf)};

	gp_json_reader *json = gp_json_reader_load(path);
	if (json)
		gp_json_read_struct(json, &val, render_ctx_desc, &ctx);

	init_fonts();

	if (!json)
		return;

	gp_json_reset(json);
	gp_json_read_struct(json, &val, render_ctx_desc, &ctx);
	gp_json_reader_finish(json);
	gp_json_reader_free(json);
}

void gp_widget_render_ctx_init(void)
{
	static uint8_t initialized;

	if (initialized)
		return;

	GP_DEBUG(1, "Initializing fonts and padding");
	render_ctx_init();
	initialized = 1;
}

static gp_backend *backend;
static gp_widget *app_layout;
static gp_dialog *cur_dialog;
static int back_from_dialog;

static void render_and_flip(gp_widget *layout, int render_flags)
{
	gp_bbox flip = {};

	ctx.flip = &flip;
	gp_widget_render(layout, &ctx, render_flags);
	ctx.flip = NULL;

	if (cur_dialog)
		gp_rect_xywh(ctx.buf, layout->x, layout->y, layout->w, layout->h, ctx.text_color);

	if (gp_bbox_empty(flip))
		return;

	GP_DEBUG(1, "Updating area " GP_BBOX_FMT, GP_BBOX_PARS(flip));

	gp_backend_update_rect_xywh(backend, flip.x, flip.y, flip.w, flip.h);
}

void __attribute__ ((visibility ("hidden"))) widget_render_refresh(void)
{
	gp_app_send_event(GP_WIDGET_EVENT_COLOR_SCHEME, &ctx);

	render_and_flip(app_layout, GP_WIDGET_REDRAW | GP_WIDGET_COLOR_SCHEME);
}

void gp_widget_render_zoom(int zoom_inc)
{
	if (ctx.font_size + zoom_inc < 5)
		return;

	ctx.font_size += zoom_inc;
	render_ctx_init();
	//TODO: Broken!
	gp_widget_render(app_layout, &ctx, GP_WIDGET_RESIZE);
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
	timers[i].period = GP_TIMER_STOP;
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

void gp_widgets_redraw(struct gp_widget *layout)
{
	if (!layout) {
		GP_DEBUG(1, "Redraw called with NULL layout!");
		return;
	}

	if (!layout->redraw && !layout->redraw_child)
		return;

	if (back_from_dialog) {
		back_from_dialog = 0;
		gp_widget_calc_size(layout, &ctx, 0, 0, 1);
		if (gp_pixmap_w(backend->pixmap) != layout->w ||
		    gp_pixmap_h(backend->pixmap) != layout->h) {
			gp_backend_resize(backend, layout->w, layout->h);
			return;
		}
	}

	if (gp_pixmap_w(backend->pixmap) < layout->w ||
	    gp_pixmap_h(backend->pixmap) < layout->h) {
		gp_backend_resize(backend, layout->w, layout->h);
		return;
	}

	if (gp_pixmap_w(backend->pixmap) == 0 ||
	    gp_pixmap_h(backend->pixmap) == 0)
		return;

	render_and_flip(layout, 0);
}

static char *backend_init_str = NULL;

void gp_widget_timer_queue_switch(gp_timer **);

static gp_task_queue task_queue = {};

void gp_widgets_task_ins(gp_task *task)
{
	if (backend)
		gp_backend_task_ins(backend, task);
	else
		gp_task_queue_ins(&task_queue, task);
}

void gp_widgets_task_rem(gp_task *task)
{
	if (backend)
		gp_backend_task_rem(backend, task);
	else
		gp_task_queue_rem(&task_queue, task);
}

static gp_dlist fds;

void gp_widget_poll_add(gp_fd *fd)
{
	if (backend)
		gp_backend_poll_add(backend, fd);
	else
		gp_dlist_push_head(&fds, &fd->lhead);
}

void gp_widget_poll_rem(gp_fd *fd)
{
	if (!backend) {
		GP_FATAL("Not implemented!");
		return;
	}

	gp_backend_poll_rem(backend, fd);
}

gp_fd *gp_widget_poll_rem_by_fd(int fd)
{
	if (!backend) {
		GP_FATAL("Not implemented!");
		return NULL;
	}

	return gp_backend_poll_rem_by_fd(backend, fd);
}

static void move_poll(gp_backend *backend)
{
	gp_dlist_head *lh;

	while ((lh = gp_dlist_pop_head(&fds)))
		gp_backend_poll_add(backend, GP_LIST_ENTRY(lh, gp_fd, lhead));
}

void gp_widgets_layout_init(gp_widget *layout, const char *win_tittle)
{
	gp_widget_render_ctx_init();

	backend = gp_backend_init(backend_init_str, 0, 0, win_tittle);
	if (!backend)
		exit(1);

	move_poll(backend);

	gp_widget_timer_queue_switch(&backend->timers);
	gp_backend_task_queue_set(backend, &task_queue);

	gp_key_repeat_timer_init(backend->event_queue, &backend->timers);

	ctx.buf = backend->pixmap;
	ctx.pixel_type = backend->pixmap->pixel_type;

	widgets_color_scheme_load();

	gp_widget_calc_size(layout, &ctx, 0, 0, 1);

	app_layout = layout;

	gp_backend_resize(backend, layout->w, layout->h);

	if (gp_pixmap_w(backend->pixmap) < layout->w ||
	    gp_pixmap_h(backend->pixmap) < layout->h) {
		return;
	}

	int flag = 0;

	if (layout->w != gp_pixmap_w(backend->pixmap) ||
	    layout->h != gp_pixmap_h(backend->pixmap)) {
		gp_fill(backend->pixmap, ctx.fill_color);
		flag = 1;
	}

	if (gp_pixmap_w(backend->pixmap) == 0 ||
	    gp_pixmap_h(backend->pixmap) == 0)
		return;

	gp_widget_render(layout, &ctx, flag);
	gp_backend_flip(backend);
}

const gp_pixmap *gp_widget_render_buffer(void)
{
	if (!backend)
		return NULL;

	return backend->pixmap;
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

static gp_widget *clipboard_requester;

void gp_widgets_clipboard_set(const char *str, size_t len)
{
	gp_backend_clipboard_set(backend, str, len);
}

char *gp_widgets_clipboard_get(void)
{
	return gp_backend_clipboard_get(backend);
}

void gp_widgets_clipboard_request(gp_widget *self)
{
	clipboard_requester = self;

	gp_backend_clipboard_request(backend);
}

void gp_widgets_clipboard_request_cancel(gp_widget *self)
{
	if (clipboard_requester == self)
		clipboard_requester = NULL;
}

static void clipboard_event(gp_event *ev)
{
	if (!clipboard_requester) {
		GP_WARN("Stray clipboard request!?");
		return;
	}

	gp_widget_input_event(clipboard_requester, &ctx, ev);

	clipboard_requester = NULL;
}

int gp_widgets_event(gp_event *ev, gp_widget *layout)
{
	int handled = 0;

	gp_handle_key_repeat_timer(ev);

	switch (ev->type) {
	case GP_EV_KEY:
		if (ev->code == GP_EV_KEY_DOWN) {
			if (gp_ev_any_key_pressed(ev, GP_WIDGET_KEYS_MOD_COLOR_SCHEME) &&
			    ev->val == GP_WIDGET_KEY_COLOR_SCHEME) {
				gp_widgets_color_scheme_toggle();
				handled = 1;
			}

			if (gp_ev_any_key_pressed(ev, GP_WIDGET_KEYS_MOD_ABOUT) &&
			    ev->val == GP_WIDGET_KEY_ABOUT) {
				gp_app_info_dialog_run();
				handled = 1;
			}

			if (gp_ev_any_key_pressed(ev, GP_WIDGET_KEYS_MOD_ZOOM)) {
				switch (ev->val) {
				case GP_WIDGET_KEY_ZOOM_IN:
					gp_widget_render_zoom(1);
					handled = 1;
				break;
				case GP_WIDGET_KEY_ZOOM_OUT:
					gp_widget_render_zoom(-1);
					handled = 1;
				break;
				}
			}

			if (gp_ev_any_key_pressed(ev, GP_WIDGET_KEYS_MOD_QUIT) &&
			    ev->val == GP_WIDGET_KEY_QUIT) {
				return 1;
			}
		}
	break;
	case GP_EV_SYS:
		switch (ev->code) {
		case GP_EV_SYS_RESIZE:
			gp_backend_resize_ack(backend);
			ctx.buf = backend->pixmap;
			gp_fill(backend->pixmap, ctx.fill_color);
			gp_widget_render(layout, &ctx, GP_WIDGET_RESIZE);
			gp_backend_flip(backend);
			handled = 1;
		break;
		case GP_EV_SYS_QUIT:
			return 1;
		break;
		case GP_EV_SYS_CLIPBOARD:
			clipboard_event(ev);
			handled = 1;
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

	if (cur_dialog) {
		if (cur_dialog->input_event)
			cur_dialog->input_event(cur_dialog, ev);
	} else {
		if (app_event_callback)
			app_event_callback(ev);
	}

	return 0;
}

int gp_widgets_process_events(gp_widget *layout)
{
	gp_event *ev;

	while ((ev = gp_backend_poll_event(backend))) {
		//gp_event_dump(&ev);
		//fflush(stdout);
		if (gp_widgets_event(ev, layout))
			gp_widgets_exit(0);
	}

	return 0;
}

static void print_options(int exit_val)
{
	gp_fonts_iter i;
	const gp_font_family *f;

	printf("Options:\n--------\n");
	printf("\t-b backend init string (pass -b help for options)\n");
	printf("\t-d layout\n\t\tenables layout debug\n");
	printf("\t-f uint\n\t\tsets the true-type font size\n");
	printf("\t-F font\n\t\tsets compiled-in font\n");
	printf("\t\tAvailable fonts:\n");

	GP_FONT_FAMILY_FOREACH(&i, f)
		printf("\t\t - %s\n", f->family_name);

	printf("\t-I input_string\n");
	printf("\t-s color_scheme\n\t\tlight or dark\n");
	printf("\t-i print app info\n");
	exit(exit_val);
}

void gp_widgets_getopt(int *argc, char **argv[])
{
	int opt;

	while ((opt = getopt(*argc, *argv, "b:d:f:F:hiI:s:")) != -1) {
		switch (opt) {
		case 'i':
			gp_app_info_print();
			exit(0);
		break;
		case 'b':
			backend_init_str = optarg;
		break;
		case 'd':
			if (!strcmp(optarg, "layout")) {
				ctx.debug_layout = 1;
			} else {
				printf("Invalid debug option '%s'\n", optarg);
				print_options(1);
			}
		break;
		case 'h':
			print_options(0);
		break;
		case 'f':
			str_font_size = optarg;
		break;
		case 'F':
			font_family = optarg;
		break;
		case 's':
			if (!strcmp(optarg, "dark")) {
				ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_DARK;
			} else if (!strcmp(optarg, "light")) {
				ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_LIGHT;
			} else {
				printf("Invalid color scheme '%s'!\n\n", optarg);
				print_options(1);
			}
		break;
		case 'I':
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

	GP_DEBUG(1, "Replacing layout %p %ux%u with %p %ux%u",
	         win_layout, win_layout->w, win_layout->h,
	         layout, layout->w, layout->h);

	gp_widget_resize(layout);
	gp_widget_redraw(layout);

	win_layout = layout;

	return ret;
}

long gp_dialog_run(gp_dialog *dialog)
{
	gp_widget *saved = gp_widget_layout_replace(dialog->layout);

	dialog->retval = 0;
	cur_dialog = dialog;

	for (;;) {
		gp_backend_wait(backend);
		gp_widgets_process_events(dialog->layout);

		if (dialog->retval) {
			cur_dialog = NULL;

			gp_widget_layout_replace(saved);

			back_from_dialog = 1;

			return dialog->retval;
		}

		gp_widgets_redraw(dialog->layout);
	}
}

void gp_widgets_main_loop(gp_widget *layout, const char *label,
                          void (*init)(int argc, char *argv[]),
                          int argc, char *argv[])
{
	if (argv)
		gp_widgets_getopt(&argc, &argv);

	gp_widgets_layout_init(layout, label);

	win_layout = layout;

	if (input_str)
		//gp_input_linux *input = gp_input_linux_by_devstr(input_str);

	if (init)
		init(argc, argv);

	for (;;) {
		gp_backend_wait(backend);
		gp_widgets_process_events(win_layout);
		gp_widgets_redraw(win_layout);
	}
}

void gp_widgets_backend_set(gp_backend *new_backend)
{
	if (!backend)
		backend = new_backend;
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
	gp_poll_clear(&backend->fds);

	exit(exit_value);
}
