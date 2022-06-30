//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_OPS_H
#define GP_WIDGET_OPS_H

#include <input/gp_event.h>
#include <utils/gp_bbox.h>
#include <utils/gp_json.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_render.h>

enum gp_widget_focus_flag {
	GP_FOCUS_OUT,
	GP_FOCUS_IN,
	GP_FOCUS_LEFT,
	GP_FOCUS_RIGHT,
	GP_FOCUS_UP,
	GP_FOCUS_DOWN,
	GP_FOCUS_NEXT,
	GP_FOCUS_PREV,
};

struct json_object;

typedef struct gp_offset {
	gp_coord x;
	gp_coord y;
} gp_offset;

enum gp_widget_render_flags {
	GP_WIDGET_REDRAW = 0x01,
	GP_WIDGET_REDRAW_CHILDREN = 0x02,
	/** Passed down when color scheme changed */
	GP_WIDGET_COLOR_SCHEME = 0x04,
	/** Layout needs to be resized */
	GP_WIDGET_RESIZE = 0x08,
};

struct gp_widget_ops {
	/**
	 * @brief Frees any additional memory a widget has allocated.
	 *
	 * If defined this function is called before widget itself is freed.
	 *
	 * @self A widget.
	 */
	void (*free)(gp_widget *self);

	/**
	 * @brief Widget input event handler.
	 *
	 * @return Returns non-zero if event was handled.
	 */
	int (*event)(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

	/**
	 * @brief Renders (changes in) widget layout.
	 *
	 * @ctx Render configuration.
	 * @offset Ofset in pixmap to draw to
	 * @flags Force redraw whole layout.
	 */
	void (*render)(gp_widget *self, const gp_offset *offset,
	               const gp_widget_render_ctx *ctx, int flags);

	/*
	 * Moves to focused widget.
	 */
	int (*focus)(gp_widget *self, int focus_dir);

	/*
	 * Moves focus to widget on x, y coordinates.
	 */
	int (*focus_xy)(gp_widget *self, const gp_widget_render_ctx *ctx,
	                 unsigned int x, unsigned int y);

	/*
	 * @brief Sets/moves focus to a child specific child widget.
	 *
	 * @self A container widget.
	 * @child A child widget to be focused.
	 *
	 * @return Non-zero if widget was focused successfuly.
	 */
	int (*focus_child)(gp_widget *self, gp_widget *child);

	/*
	 * Called once to calculate minimal widget sizes.
	 */
	unsigned int (*min_w)(gp_widget *self, const gp_widget_render_ctx *ctx);
	unsigned int (*min_h)(gp_widget *self, const gp_widget_render_ctx *ctx);

	/**
	 * @brief Recursively distributes widgets in a widget container.
	 *
	 * Implemented only for non-leaf widgets.
	 *
	 * @self   Widget layout to be distributed.
	 * @ctx    Render configuration.
	 * @new_wh Force distribute size on layout size change.
	 */
	void (*distribute_size)(gp_widget *self,
	                        const gp_widget_render_ctx *ctx,
	                        int new_wh);

	/**
	 * @brief A callback to iterate over all widget children.
	 *
	 * @self A widget.
	 * @func A function to be called on each child widget.
	 */
	void (*for_each_child)(gp_widget *self, void (*func)(gp_widget *child));

	/*
	 * json_object -> widget converter.
	 */
	gp_widget *(*from_json)(gp_json_buf *json, gp_json_val *val, gp_widget_json_ctx *ctx);

	/* id used for JSON loader */
	const char *id;
};

const struct gp_widget_ops *gp_widget_ops(gp_widget *self);

const struct gp_widget_ops *gp_widget_ops_by_id(const char *id);

const char *gp_widget_type_id(gp_widget *self);

const char *gp_widget_type_name(enum gp_widget_type type);

unsigned int gp_widget_min_w(gp_widget *self, const gp_widget_render_ctx *ctx);

unsigned int gp_widget_min_h(gp_widget *self, const gp_widget_render_ctx *ctx);

unsigned int gp_widget_align(gp_widget *self);

int gp_widget_input_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

void gp_widget_ops_render(gp_widget *self, const gp_offset *offset,
                          const gp_widget_render_ctx *ctx, int flags);

/*
 * Send event to a widget.
 *
 * @self A widget to send the event to.
 * @ctx A render context.
 * @ev An input event.
 *
 * @return Zero if event wasn't handled, non-zero otherwise.
 */
int gp_widget_ops_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

/*
 * Send event to a widget with a cursor offset.
 *
 * Same as gp_widget_ops_event() but with offset to the absolute coordinates is
 * applied so that the event is relative to the cursor coordinates.
 *
 * @self A widget to send the event to.
 * @ctx A render context.
 * @ev An input event.
 * @off_x X cursor offset.
 * @off_y Y cursor offset.
 *
 * @return Zero if event wasn't handled, non-zero otherwise.
 */
int gp_widget_ops_event_offset(gp_widget *self, const gp_widget_render_ctx *ctx,
                               gp_event *ev, gp_size off_x, gp_size off_y);

/**
 * @brief Moves focus, if possible, in the direction requested by the focus_dir.
 *
 * @self A widget layout.
 * @focus_dir Direction to move the focus to.
 *
 * @return Zero if focus couldn't be moved, non-zero otherwise.
 */
int gp_widget_ops_render_focus(gp_widget *self, int focus_dir);

/**
 * @brief Tries to focus a widget on a given coordinates in a layout.
 *
 * @self A widget layout.
 * @ctx A render context.
 * @x X coordinate.
 * @y Y coordinate.
 *
 * @return Zero if focus wasn't changed, non-zero otherwise.
 */
int gp_widget_ops_render_focus_xy(gp_widget *self, const gp_widget_render_ctx *ctx,
                                  unsigned int x, unsigned int y);

/**
 * @brief Moves focus to a particular widget.
 *
 * Traverses the widget layout tree to the top and sets the focus accordingly.
 *
 * @self A widget to be focused.
 *
 * @return Zero if focus couldn't be changed, non-zero otherwise.
 */
int gp_widget_ops_focus_widget(gp_widget *self);

void gp_widget_ops_distribute_size(gp_widget *self, const gp_widget_render_ctx *ctx,
                                   unsigned int w, unsigned int h, int new_wh);

/**
 * @brief Calls a callback on each child widget.
 *
 * All non-leaf widgets must implement for_each_child() callback in its ops
 * which is then called by this function.
 *
 * This function is no-op for NULL self and non-leaf widgets.
 *
 * @self A widget.
 * @func A function callback
 */
void gp_widget_ops_for_each_child(gp_widget *self, void (*func)(gp_widget *child));

/**
 * @brief Marks an area to be blit on the screen from a buffer.
 *
 * This function is called by widgets so that the render knows which parts of
 * the screen has to be updated after the call to the render function.
 *
 */
static inline void gp_widget_ops_blit(const gp_widget_render_ctx *ctx,
                                      gp_coord x, gp_coord y,
                                      gp_size w, gp_size h)
{
	if (!ctx->flip)
		return;

	if (gp_bbox_empty(*ctx->flip))
		*ctx->flip = gp_bbox_pack(x, y, w, h);
	else
		*ctx->flip = gp_bbox_merge(*ctx->flip, gp_bbox_pack(x, y, w, h));
}

/**
 * @brief Returns true if widget should be repainted.
 *
 * @self A widget.
 * @flags Render flags passed down to the widget->render() function.
 *
 * @return Non-zero if widget should be repainted.
 */
static inline int gp_widget_should_redraw(gp_widget *self, int flags)
{
	return self->redraw || (flags & GP_WIDGET_REDRAW);
}

/**
 * @brief Calculates layout size recursively.
 *
 * The size may end up larger than WxH if there is too much widgets or smaller
 * than WxH if align is not set to fill.
 *
 * @layout Widget layout.
 * @ctx    Render context, e.g. fonts, pixel type, padding size, etc.
 * @w      Width we are trying to fit into
 * @h      Height we are trying to fit into
 * @new_wh Force to position widgets on changed layout size
 */
void gp_widget_calc_size(gp_widget *layout, const gp_widget_render_ctx *ctx,
                         unsigned int w, unsigned int h, int new_wh);

/**
 * @brief Redraw widget.
 *
 * Marks widget to be repainted on next update.
 *
 * @self A widget.
 */
void gp_widget_redraw(gp_widget *self);

/**
 * @brief Resize widget.
 *
 * Marks widget to be resized on next update. The resize is propagated in the
 * layout to the top. E.g. if widget has to grow, the whole layout will.
 *
 * @self A widget.
 */
void gp_widget_resize(gp_widget *self);

/**
 * @brief Redraw all child widgets.
 *
 * Marks all children to be repainted on next update. This is used internally
 * by container widgets when layout has changed, e.g. tab has changed.
 *
 * @self A container widget.
 */
void gp_widget_redraw_children(gp_widget *self);

/*
 * Resizes and redraws changed widgets.
 */
void gp_widget_render(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh);

#endif /* GP_WIDGET_OPS_H */
