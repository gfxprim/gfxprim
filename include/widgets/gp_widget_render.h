//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_RENDER_H__
#define GP_WIDGET_RENDER_H__

#include <core/gp_core.h>
#include <gfx/gp_gfx.h>
#include <text/gp_text.h>
#include <input/gp_timer.h>
#include <utils/gp_bbox.h>

#include <widgets/gp_widget.h>

typedef struct gp_widget_render_ctx {
	/* colors */
	gp_pixel text_color;
	gp_pixel bg_color;
	gp_pixel fg_color;
	gp_pixel fg2_color;
	gp_pixel sel_color;
	gp_pixel alert_color;

	/* fonts */
	gp_text_style *font;
	gp_text_style *font_bold;

	gp_text_style *font_big;
	gp_text_style *font_big_bold;

	/* pixel type used for drawing */
	gp_pixel_type pixel_type;

	/* padding between widgets */
	unsigned int padd;

	/* area to update on a screen after a call to gp_widget_render() */
	gp_bbox *flip;

	/* passed down if only part of the layout has to be rendered */
	gp_bbox *bbox;

	gp_pixmap *buf;
} gp_widget_render_ctx;

void gp_widget_render_init(void);

typedef struct gp_widget_timer {
	uint32_t (*callback)(void *priv);
	void *priv;
	/* do not touch */
	gp_timer tmr;
} gp_widget_timer;

enum gp_widget_render_timer_flags {
	GP_TIMER_RESCHEDULE = 0x01,
};

extern struct gp_fds *gp_widgets_fds;

void gp_widget_render_timer(gp_widget *self, int flags, unsigned int timeout_ms);
void gp_widget_render_timer_cancel(gp_widget *self);

/**
 * @brief Returns a pointer to the current render context.
 *
 * @return A render context.
 */
const gp_widget_render_ctx *gp_widgets_render_ctx(void);

/**
 * @brief Parses options, returns positional arguments, e.g. paths.
 *
 * @argc A pointer to main() arguments count.
 * @argv A pointer to an main() arguments array.
 */
void gp_widgets_getopt(int *argc, char **argv[]);

/**
 * @brief Register application event callback.
 *
 * All input events that are not handled by the widget library are passed to
 * the callback registered by this function.
 *
 * @on_event An event callback.
 */
void gp_widgets_register_callback(int (*on_event)(gp_event *));

/**
 * @brief Widgets main loop.
 *
 * Renders a widget layout on a screen or into an window, handles input events,
 * etc. This function does not return.
 *
 * @layout A widget layout to show.
 * @label Application label.
 * @init An init function.
 * @argc A main() argc.
 * @argv A main() argv.
 */
void gp_widgets_main_loop(struct gp_widget *layout, const char *label,
                          void (*init)(void), int argc, char *argv[])
                          __attribute__((noreturn));

/**
 * @brief Replace a application layout.
 *
 * @layout New application widget layout.
 *
 * @return An old application layout.
 */
gp_widget *gp_widget_layout_replace(gp_widget *layout);

/*
 * TODO: Obsolete?
 */
void gp_widgets_redraw(gp_widget *layout);

/*
 * TODO: Obsolete?
 */
int gp_widgets_process_events(gp_widget *layout);

/*
 * TODO: Obsolete?
 */
void gp_widgets_layout_init(gp_widget *layout, const char *win_tittle);

/**
 * @brief Increases/decreases font sizes, etc.
 *
 * TODO: Broken.
 *
 * @zoom_inc A zoom increment.
 */
void gp_widget_render_zoom(int zoom_inc);

#endif /* GP_WIDGET_RENDER_H__ */
