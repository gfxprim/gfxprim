//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2026 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_ops.h
 * @brief Widget internals.
 *
 * You will not need to understand this part of the library unless you need to
 * add a custom widget types.
 */

#ifndef GP_WIDGET_OPS_H
#define GP_WIDGET_OPS_H

#include <input/gp_event.h>
#include <utils/gp_bbox.h>
#include <utils/gp_json.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_render.h>

struct json_object;

typedef struct gp_offset {
	gp_coord x;
	gp_coord y;
} gp_offset;

/** @brief Widget rendering flags. */
enum gp_widget_render_flags {
	/**
	 * @brief Widget needs to be repainted.
	 *
	 * Set by the widget to request a repaint next time the main loop
	 * returns to the library.
	 */
	GP_WIDGET_REDRAW = 0x01,
	GP_WIDGET_REDRAW_CHILDREN = 0x02,
	/**
	 * @brief A color scheme has changed.
	 *
	 * Passed down from the widget render when color scheme changed and
	 * everything has to be repainted.
	 */
	GP_WIDGET_COLOR_SCHEME = 0x04,
	/**
	 * @brief Layout needs to be resized.
	 *
	 * TODO description.
	 */
	GP_WIDGET_RESIZE = 0x08,
	/**
	 * @brief Widget is disabled.
	 *
	 * When a widget is disabled all it's children gets this flag passed
	 * down to their render functions.
	 */
	GP_WIDGET_DISABLED = 0x10,
};

/**
 * @brief Callbacks that implements a widget.
 *
 * Container widgets, i.e. widgets with children, implement callbacks to access
 * its children that are otherwise NULL. These are mostly functions to move
 * focus, distribute any leftover size to the layout, and iterate over widgets.
 * Also most of the callbacks for container widgets are recursive, e.g. the
 * JSON parser internally calls gp_widget_from_json() to parse it's children.
 *
 * @warning Never call these functions directly use the wrappers defined below.
 */
struct gp_widget_ops {
	/**
	 * @brief Frees any additional memory a widget has allocated.
	 *
	 * If defined this function is called before widget itself is freed.
	 *
	 * @param self A widget.
	 */
	void (*free)(gp_widget *self);

	/**
	 * @brief Widget input event handler.
	 *
	 * May be NULL for widgets that do not consume input events.
	 *
	 * @param self A widget.
	 * @param ctx A widget render context.
	 * @param ev An input event to be processed by the widget.
	 * @return Returns non-zero if event was handled.
	 */
	int (*event)(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

	/**
	 * @brief Renders (changes in) widget layout.
	 *
	 * @param ctx A widget render context.
	 * @param offset Ofset in pixmap to draw to
	 * @param flags Flags enum gp_widget_render_flags.
	 */
	void (*render)(gp_widget *self, const gp_offset *offset,
	               const gp_widget_render_ctx *ctx, int flags);

	/**
	 * @brief Returns a focused child of a container.
	 *
	 * Containers implement the focus_child_* callbacks, all the focus
	 * logic is implemented in the widget ops code. The callbacks only
	 * store and look up the child that is on the focus path. Focus is a
	 * path from the layout root to a leaf widget and the widget::focused
	 * flag is set on all widgets on that path, containers included. The
	 * flag is maintained by the widget ops code and must not be modified
	 * by the widget implementations.
	 *
	 * Containers that can be focused themselves, e.g. tabs with a focused
	 * title, use the self pointer in place of a child to describe that.
	 *
	 * The focused child is remembered even after the container lost the
	 * focus, which is used when the focus moves back into the container.
	 *
	 * @param self A container widget.
	 *
	 * @return A focused child, self if the container itself is focused or
	 *         NULL if there is none.
	 */
	gp_widget *(*focus_child_get)(gp_widget *self);

	/**
	 * @brief Sets a focused child of a container.
	 *
	 * This only stores the child, no focus events have to be sent and the
	 * widget::focused flags must not be modified.
	 *
	 * @param self A container widget.
	 * @param child A child or self to be focused.
	 *
	 * @return True on success, false if the child cannot be focused,
	 *         e.g. it's not in an active tab.
	 */
	bool (*focus_child_set)(gp_widget *self, gp_widget *child);

	/**
	 * @brief Returns a next child in a direction.
	 *
	 * The widget ops code calls this repeatedly until it finds a child that
	 * can be focused or NULL is returned.
	 *
	 * Optional, containers with a single child, which is returned by the
	 * focus_child_get(), do not have to implement it.
	 *
	 * @param self A container widget.
	 * @param from A child to start from, NULL when the focus moves into the
	 *             container, i.e. returns first child in the direction.
	 * @param focus_dir An enum gp_widget_focus_flag.
	 *
	 * @return A child, self or NULL if there is no next child.
	 */
	gp_widget *(*focus_child_next)(gp_widget *self, gp_widget *from,
	                               int focus_dir);

	/**
	 * @brief Returns a child on x, y coordinates.
	 *
	 * @param self A container widget.
	 * @param ctx A render context.
	 * @param x A pointer to x coordinate.
	 * @param y A pointer to y coordinate.
	 *
	 * The x and y coordinates passed to the call are relative to the
	 * container widget and are normalized to be relative to the returned
	 * focused widget upon return with a succesful lookup.
	 *
	 * @return A child, self or NULL if there is nothing focusable on x, y.
	 */
	gp_widget *(*focus_child_xy)(gp_widget *self, const gp_widget_render_ctx *ctx,
	                             unsigned int *x, unsigned int *y);

	/**
	 * @brief Calculates a minimal widget width.
	 *
	 * Container widgets call these functions recursively for its children,
	 * then the minimal children sizes to compute the overall minimal size.
	 *
	 * @param self A widget.
	 * @param ctx A widget rendering context.
	 * @return A minimal widget width in pixels.
	 */
	unsigned int (*min_w)(gp_widget *self, const gp_widget_render_ctx *ctx);
	/**
	 * @brief Calculates a minimal widget height.
	 *
	 * Container widgets call these functions recursively for its children,
	 * then the minimal children sizes to compute the overall minimal size.
	 *
	 * @param self A widget.
	 * @param ctx A widget rendering context.
	 * @return A minimal widget height in pixels.
	 */
	unsigned int (*min_h)(gp_widget *self, const gp_widget_render_ctx *ctx);

	/**
	 * @brief Recursively distributes widgets in a widget container.
	 *
	 * Implemented only for non-leaf widgets.
	 *
	 * @param self Widget layout to be distributed.
	 * @param ctx Render configuration.
	 * @param new_wh Force distribute size on layout size change.
	 */
	void (*distribute_w)(gp_widget *self,
	                     const gp_widget_render_ctx *ctx,
	                     int new_wh);

	void (*distribute_h)(gp_widget *self,
	                     const gp_widget_render_ctx *ctx,
	                     int new_wh);

	/**
	 * @brief A callback to iterate over all widget children.
	 *
	 * @param self A widget.
	 * @param func A function to be called on each child widget.
	 * @param priv A private pointer to pass down a state.
	 */
	void (*for_each_child)(gp_widget *self,
	                       void (*func)(gp_widget *child, void *priv),
	                       void *priv);

	/**
	 * @brief A JSON to widget parser.
	 *
	 * Reads a serialized widget from JSON.
	 *
	 * @param json A JSON reader.
	 * @param val A JSON value.
	 * @param ctx A widget JSON parser context.
	 */
	gp_widget *(*from_json)(gp_json_reader *json, gp_json_val *val, gp_widget_json_ctx *ctx);

	/**
	 * @brief A widget id, e.g. "checkbox".
	 */
	const char *id;
};

/**
 * @brief Returns widget ops structure.
 *
 * @param self A widget.
 *
 * @return A pointer to widgets ops structure.
 */
const struct gp_widget_ops *gp_widget_ops(gp_widget *self);

const struct gp_widget_ops *gp_widget_ops_by_id(const char *id);

const char *gp_widget_type_id(gp_widget *self);

const char *gp_widget_type_name(enum gp_widget_type type);

unsigned int gp_widget_min_w(gp_widget *self, const gp_widget_render_ctx *ctx);

unsigned int gp_widget_min_h(gp_widget *self, const gp_widget_render_ctx *ctx);

unsigned int gp_widget_align(gp_widget *self);

/**
 * @brief Returns true if widget accepts input events.
 *
 * A widget handles input events if all are true:
 *
 * - the widget is not disabled
 * - it has an input event handler
 * - the handler is enabled, i.e. 'no_events' flag is not set
 *
 * Note that this will return true for widgets that have a disabled parent and
 * are rendered disabled.
 *
 * @param self A widget.
 * @return True if widget accepts input events.
 */
static inline bool gp_widget_accepts_input_events(gp_widget *self)
{
	const struct gp_widget_ops *ops = gp_widget_ops(self);

	if (self->no_events || self->disabled)
		return false;

	return ops->event != NULL;
}

int gp_widget_input_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

void gp_widget_ops_render(gp_widget *self, const gp_offset *offset,
                          const gp_widget_render_ctx *ctx, int flags);

/**
 * @brief Send an event to a widget.
 *
 * @param self A widget to send the event to.
 * @param ctx A render context.
 * @param ev An input event.
 *
 * @return Zero if event wasn't handled, non-zero otherwise.
 */
int gp_widget_ops_event(gp_widget *self, const gp_widget_render_ctx *ctx, gp_event *ev);

/**
 * @brief Send event to a widget with a cursor offset.
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

void gp_widget_ops_distribute_w(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int w, int new_wh);
void gp_widget_ops_distribute_h(gp_widget *self, const gp_widget_render_ctx *ctx, unsigned int h, int new_wh);

/**
 * @brief Calls a callback on each child widget.
 *
 * All non-leaf widgets must implement for_each_child() callback in its ops
 * which is then called by this function.
 *
 * This function is no-op for NULL self and leaf widgets.
 *
 * @param self A widget.
 * @param func A function callback.
 */
void gp_widget_ops_for_each_child(gp_widget *self,
                                  void (*func)(gp_widget *child));

/**
 * @brief Calls a callback on each child widget.
 *
 * All non-leaf widgets must implement for_each_child() callback in its ops
 * which is then called by this function.
 *
 * This function is no-op for NULL self and leaf widgets.
 *
 * @param self A widget.
 * @param func A function callback.
 * @param priv A private pointer to pass down a state.
 */
void gp_widget_ops_for_each_child_priv(gp_widget *self,
                                       void (*func)(gp_widget *child, void *priv),
                                       void *priv);

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

static inline int gp_widget_is_disabled(gp_widget *self, int flags)
{
	return self->disabled || (flags & GP_WIDGET_DISABLED);
}

/**
 * @brief Calculates layout size recursively.
 *
 * The size may end up larger than WxH if there is too much widgets or smaller
 * than WxH if align is not set to fill.
 *
 * @param layout Widget layout.
 * @param ctx Render context, e.g. fonts, pixel type, padding size, etc.
 * @param w Width we are trying to fit into
 * @param h Height we are trying to fit into
 * @param new_wh If set the layout size is recalculated, i.e. the layout size has changed.
 */
void gp_widget_calc_size(gp_widget *layout, const gp_widget_render_ctx *ctx,
                         unsigned int w, unsigned int h, int new_wh);

/**
 * @brief Requests widget repaint.
 *
 * Marks widget to be repainted on next update.
 *
 * @param self A widget.
 */
void gp_widget_redraw(gp_widget *self);

/**
 * @brief Requests widget resize.
 *
 * Marks widget to be resized on next update. The resize is propagated in the
 * layout to the top. E.g. if widget has to grow, the whole layout will.
 *
 * @param self A widget.
 */
void gp_widget_resize(gp_widget *self);

/**
 * @brief Redraw all child widgets.
 *
 * Marks all children to be repainted on next update. This is used internally
 * by container widgets when layout has changed, e.g. visible tab has changed.
 *
 * @param self A container widget.
 */
void gp_widget_redraw_children(gp_widget *self);

/**
 * @brief Resizes and redraws changed widgets.
 *
 * @param self A widget layout.
 * @param ctx A rendering context.
 * @param new_wh If set the layout size is recalculated, i.e. the layout size has changed.
 */
void gp_widget_render(gp_widget *self, const gp_widget_render_ctx *ctx, int new_wh);

#endif /* GP_WIDGET_OPS_H */
