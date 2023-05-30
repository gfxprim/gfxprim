//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_RENDER_H
#define GP_WIDGET_RENDER_H

#include <core/gp_core.h>
#include <gfx/gp_gfx.h>
#include <text/gp_text.h>
#include <utils/gp_timer.h>
#include <utils/gp_bbox.h>
#include <utils/gp_fds.h>

#include <widgets/gp_widget_types.h>
#include <widgets/gp_widgets_color_scheme.h>

struct gp_widget_render_ctx {
	gp_pixmap *buf;

	enum gp_widgets_color_scheme color_scheme;

	/* colors */
	union {
		gp_pixel colors[GP_WIDGETS_THEME_COLORS + 16];
		struct {
			gp_pixel text_color;
			gp_pixel fg_color;
			gp_pixel bg_color;
			gp_pixel hl_color;
			gp_pixel sel_color;
			gp_pixel alert_color;
			gp_pixel warn_color;
			gp_pixel accept_color;
			gp_pixel fill_color;
			gp_pixel col_disabled;
			/* 16 colors */
			gp_pixel black;
			gp_pixel red;
			gp_pixel green;
			gp_pixel yellow;
			gp_pixel blue;
			gp_pixel magenta;
			gp_pixel cyan;
			gp_pixel gray;
			gp_pixel br_black;
			gp_pixel br_red;
			gp_pixel br_green;
			gp_pixel br_yellow;
			gp_pixel br_blue;
			gp_pixel br_magenta;
			gp_pixel br_cyan;
			gp_pixel white;
		} __attribute__((packed));
	};

	/* fonts */
	gp_text_style *font;
	gp_text_style *font_bold;

	gp_text_style *font_big;
	gp_text_style *font_big_bold;

	gp_text_style *font_mono;
	gp_text_style *font_mono_bold;

	/* pixel type used for drawing */
	gp_pixel_type pixel_type;

	/* padding between widgets */
	uint8_t padd;
	/* frame thickness 0 == 1px */
	uint8_t fr_thick;
	/* frame roundness */
	uint8_t fr_round;
	/* cursor thickness 0 == 1px */
	uint8_t cur_thick;
	/* font size */
	uint8_t font_size;
	/* maximal delay between two clicks for a double click */
	uint16_t dclick_ms;

	/* area to update on a screen after a call to gp_widget_render() */
	gp_bbox *flip;

	/* passed down if only part of the layout has to be rendered */
	gp_bbox *bbox;

	/* draws rectangles around container widgets */
	int debug_layout:1;
};

static inline int gp_widgets_is_dclick(uint64_t time_now, uint64_t time_prev,
                                       const gp_widget_render_ctx *ctx)
{
	if (time_now - time_prev < (uint64_t)ctx->dclick_ms)
		return 1;

	return 0;
}

/*
 * @brief Initializes render context fonts and padding.
 *
 * Colors are not initialized until we have a pixmap to render to since we do
 * not know the pixel format until that time.
 */
void gp_widget_render_ctx_init(void);

/**
 * @brief Returns a pointer to a render pixmap.
 *
 * @return Pointer to a render pixmap or NULL if not available.
 */
const gp_pixmap *gp_widget_render_buffer(void);

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
                          void (*init)(int argc, char *argv[]),
                          int argc, char *argv[])
                          __attribute__((noreturn));

/**
 * @brief Exits the appliaction.
 *
 * @exit_value Exit value passed to exit().
 */
void gp_widgets_exit(int exit_value) __attribute__((noreturn));

/**
 * @brief Replace a application layout.
 *
 * @layout New application widget layout.
 *
 * @return An old application layout.
 */
gp_widget *gp_widget_layout_replace(gp_widget *layout);

/**
 * @brief Runs a dialog.
 *
 * Temporarily replaces layout shown on the screen with a dialog.
 *
 * @dialog A dialog to be shown to the user.
 *
 * @return A return value from a dialog.
 */
long gp_dialog_run(gp_dialog *dialog);

/**
 * @brief Sets the clipboard data.
 *
 * @str A clipboard string.
 * @len Optional size limit, if set to 0 whole string is used.
 */
void gp_widgets_clipboard_set(const char *str, size_t len);

/**
 * @brief Returns clipboard data.
 *
 * Returns valid data only after GP_EV_SYS_CLIPBOARD event has been received.
 *
 * @reuturn A clipboad data buffer allocated by malloc(), has to be freed by
 * the caller.
 */
char *gp_widgets_clipboard_get(void);

/**
 * @brief Request clipboard data.
 *
 * When data are ready the widget gets GP_EV_SYS_CLIPBOARD event.
 *
 * @self A widget which requests clipboard data.
 */
void gp_widgets_clipboard_request(gp_widget *self);

/**
 * @brief Cancel clipboard data request.
 *
 * Call this to cancel clipboard requests, this is useful when widget that may
 * have in-flight clipboard request is being destroyed.
 *
 * @self A widget which cancels the request.
 */
void gp_widgets_clipboard_request_cancel(gp_widget *self);

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

#endif /* GP_WIDGET_RENDER_H */
