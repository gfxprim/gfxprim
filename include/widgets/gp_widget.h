//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget.h
 * @brief A widget implementation base.
 */

#ifndef GP_WIDGET_H
#define GP_WIDGET_H

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <core/gp_debug.h>
#include <utils/gp_types.h>
#include <widgets/gp_common.h>
#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_disable.h>

/** @brief A widget base. */
struct gp_widget {
	/**
	 * @brief A widget type enum #gp_widget_type.
	 *
	 * Widget type e.g. button or text box. The type is used to locate the
	 * struct #gp_widget_ops in a global widget ops table.
	 *
	 * It's also used to assert that we are dealing with a correct widget
	 * in the API functions, e.g. button functions check that the widget
	 * type is set to button at the start of each function.
	 */
	unsigned int type;
	/**
	 * @brief A widget class enum #gp_widget_class.
	 *
	 * Widget classes define widgets with exacly same API but different UI
	 * and implementation. E.g. GP_WIDGET_CLASS_CHOICE can be rendered as a
	 * radio button or a spin button.
	 */
	unsigned int widget_class;
	/**
	 * @brief Parent widget in the widget tree.
	 *
	 * It's set to NULL unless widget has been inserted into a container
	 * widget, e.g. grid, tabs, etc.
	 */
	gp_widget *parent;
	/**
	 * @brief An application event handler.
	 *
	 * This function is supplied by the application. Changes in widget
	 * state are propagated to the application by this function.
	 */
	int (*on_event)(gp_widget_event *);

	/**
	 * @brief A pointer to arbitrary application data.
	 *
	 * Useful for event handlers and other user defined functions. The
	 * library will not access or modify the memory pointed to by this.
	 */
	void *priv;

	/**
	 * @brief A relative offset to the parent widget in pixels.
	 */
	unsigned int x, y;

	/**
	 * @brief Current widget size in pixels.
	 */
	unsigned int w, h;

	/**
	 * @brief Cached widget minimal size in pixel.
	 */
	unsigned int min_w, min_h;

	/** @brief Widget alignment in parent container, enum #gp_widget_alignment. */
	unsigned int align:16;

	/**
	 * @brief Widget no-shrink flag.
	 *
	 * If set widget will not shrink, i.e. will be resized only when the
	 * new minimal size is bigger than the previous one.
	 *
	 * By default this is set to 0.
	 */
	unsigned int no_shrink:1;
	unsigned int no_resize:1;

	/**
	 * @brief Internal widget repaint flag.
	 *
	 * This flag is used internally by the widget code to mark it to be
	 * repainted.
	 */
	unsigned int redraw:1;
	/*
	 * If set there is a child widget to be repainted, the widget_ops_render()
	 * function is called but the widget itself shouldn't be repainted.
	 */
	unsigned int redraw_child:1;
	/*
	 * Redraw whole subtree, i.e. all children and their children, etc.
	 */
	unsigned int redraw_children:1;

	/**
	 * @brief Set if widget is focused.
	 */
	unsigned int focused:1;

	/**
	 * @brief Internal widget resize flag.
	 *
	 * If set widget resize is schedulled before we attemp to repaint it.
	 */
	unsigned int resized:1;

	/**
	 * @brief Internal widget flag.
	 *
	 * If set the widget cannot get focused and will not process input
	 * events even if ops->event is implemented.
	 */
	unsigned int no_events:1;

	/**
	 * @brief Internal disabled flag.
	 *
	 * If set widget is 'grayed out' and no events are processed.
	 */
	unsigned int disabled:1;

	/**
	 * @brief A mask to enable and disable a widget events.
	 *
	 * This mask decides for which kind of enum #gp_widget_event_type will
	 * be the gp_widget::on_event called.
	 */
	uint32_t event_mask;

	union {
		struct gp_widget_grid *grid;
		struct gp_widget_tabs *tabs;

		struct gp_widget_bool *b;
		struct gp_widget_bool *button;
		struct gp_widget_bool *checkbox;

		struct gp_widget_label *label;

		struct gp_widget_int *i;
		struct gp_widget_pbar *pbar;
		struct gp_widget_int *spin;
		struct gp_widget_int *slider;

		struct gp_widget_tbox *tbox;

		struct gp_widget_choice *choice;

		struct gp_widget_table *tbl;

		struct gp_widget_pixmap *pixmap;

		struct gp_widget_stock *stock;

		struct gp_widget_scroll_area *scroll;

		struct gp_widget_frame *frame;

		struct gp_widget_markup *markup;

		struct gp_widget_switch *switch_;

		struct gp_widget_overlay *overlay;

		struct gp_widget_log *log;

		struct gp_widget_graph *graph;

		void *payload;
	};
	char buf[];
};

/** @brief Widget types. */
enum gp_widget_type {
	/** @brief A container widget to create layout. */
	GP_WIDGET_GRID,
	/** @brief A tabs widget. */
	GP_WIDGET_TABS,
	/** @brief A button widget. */
	GP_WIDGET_BUTTON,
	/** @brief A checkbox widget. */
	GP_WIDGET_CHECKBOX,
	/** @brief A label widget. */
	GP_WIDGET_LABEL,
	/** @brief A progress bar widget. */
	GP_WIDGET_PROGRESSBAR,
	/** @brief A spinner widget. */
	GP_WIDGET_SPINNER,
	/** @brief A slider widget. */
	GP_WIDGET_SLIDER,
	/** @brief A textbox widget. */
	GP_WIDGET_TBOX,
	/** @brief A radiobutton widget. */
	GP_WIDGET_RADIOBUTTON,
	/** @brief A spinnbutton widget. */
	GP_WIDGET_SPINBUTTON,
	/** @brief A table widget. */
	GP_WIDGET_TABLE,
	/** @brief A pixmap widget. */
	GP_WIDGET_PIXMAP,
	/** @brief A stock image widget. */
	GP_WIDGET_STOCK,
	/** @brief A scroll area widget. */
	GP_WIDGET_SCROLL_AREA,
	/** @brief A frame widget. */
	GP_WIDGET_FRAME,
	/** @brief A markup widget. */
	GP_WIDGET_MARKUP,
	/** @brief A layout switch widget. */
	GP_WIDGET_SWITCH,
	/** @brief An overlay widget. */
	GP_WIDGET_OVERLAY,
	/** @brief A widget log widget. */
	GP_WIDGET_LOG,
	/** @brief A graph widget. */
	GP_WIDGET_GRAPH,
	/** @brief A numeber of widgets. */
	GP_WIDGET_MAX,
};

/**
 * @brief Widget classes.
 *
 * Class widgets are operated on by a class functions rather than widget specific functions.
 */
enum gp_widget_class {
	/** @brief Widget has no class, most common case. */
	GP_WIDGET_CLASS_NONE = 0,
	/**
	 * @brief Boolean class.
	 *
	 * The widget has two values, e.g. checkbox.
	 *
	 * See gp_widget_class_bool.h for the list of functions.
	 */
	GP_WIDGET_CLASS_BOOL,
	/**
	 * @brief An integer widget with minimum and maximun.
	 *
	 * See gp_widget_class_int.h for the list of functions.
	 */
	GP_WIDGET_CLASS_INT,
	/**
	 * @brief A widget to choose a single element from a set.
	 *
	 * See gp_widget_class_choice.h for the list of functions.
	 */
	GP_WIDGET_CLASS_CHOICE,
	GP_WIDGET_CLASS_MAX,
};

/**
 * @brief Returns widget class name.
 *
 * @param widget_class A widget class.
 *
 * @return A widget class name.
 */
const char *gp_widget_class_name(enum gp_widget_class widget_class);

/**
 * @brief A widget alignment.
 *
 * Defines a widget alignment in the parent container, the bottom half of the
 * byte defines horizontal alignment, the top half vertical alignment.
 *
 * The widgets are organized in a two dimensional tree where each widget/layer
 * is an rectangle in a plane. The rectanles on a given tree layer are distinct
 * and the rectanle on an upper layer contains all rectangles on lower layer.
 *
 * The widget layout is computed in two steps, first minimal size is computed
 * recursively from the top level widget down to the leaf widgets, then if the
 * window is bigger than the minimal needed size, the leftover space is being
 * distributed between the widgets.
 *
 * In order for a widget to take more space than the minimal size, i.e. be
 * resizable the horizontal and/or vertical alignment has to be set to fill.
 * Which especially means that layout can be resized only and only if the top
 * level layout widget is resizable. Apart from fill each widget can be set to
 * be positioned top/center/bottom vertically as well as left/center/right
 * horizontally.
 *
 * Examples
 * --------
 * Grid horizontal and vertical alignment set to fill button to center
 *
 * @image html grid_fill_button_center.png
 *
 * Widget layout in JSON:
 * @code{.json}
 * {
 *  "info": {"version": 1, "license": "GPL-2.0-or-later"},
 *  "layout": {
 *   "align": "fill",
 *   "widgets": [
 *    {"type": "button", "label": "Button", "align": "center"}
 *   ]
 *  }
 * }
 * @endcode
 *
 * Horizontal and vertical alignment set to fill for both
 *
 * @image html grid_fill_button_fill.png
 *
 * Widget layout in JSON:
 * @code{.json}
 * {
 *  "info": {"version": 1, "license": "GPL-2.0-or-later"},
 *  "layout": {
 *   "align": "fill",
 *   "widgets": [
 *    {"type": "button", "label": "Button", "align": "fill"}
 *   ]
 *  }
 * }
 * @endcode
 *
 * Horizontal and vertical alignment set to center for grid
 *
 * The button alignment does not matter in this case, since it exaclty fits its
 * assigned space.
 *
 * @image html grid_center_button.png
 *
 * Widget layout in JSON:
 * @code{json}
 * {
 *  "info": {"version": 1, "license": "GPL-2.0-or-later"},
 *  "layout": {
 *   "align": "center",
 *   "widgets": [
 *    {"type": "button", "label": "Button"}
 *   ]
 *  }
 * }
 * @endcode
 */
enum gp_widget_alignment {
	/** @brief Center horizontally. */
	GP_HCENTER = 0x01,
	/** @brief Align to the left. */
	GP_LEFT    = 0x02,
	/** @brief Align to the right. */
	GP_RIGHT   = 0x03,
	/** @brief Fill available horizontal space, will strech the widget. */
	GP_HFILL   = 0x08,

	/** @brief Center vertically. */
	GP_VCENTER = 0x10,
	/** @brief Align to the top. */
	GP_TOP     = 0x20,
	/** @brief Align to the bottom. */
	GP_BOTTOM  = 0x30,
	/** @brief Fill available vertical space, will strech the widget. */
	GP_VFILL   = 0x80,

	/** @brief Horizontal alignment mask. */
	GP_HALIGN_MASK = 0x0f,
	/** @brief Vertical alignment mask. */
	GP_VALIGN_MASK = 0xf0,
	/** @brief Shortcut for setting both hfill and vfill. */
	GP_FILL = GP_VFILL | GP_HFILL,

	GP_HCENTER_WEAK = 0x00,
	GP_VCENTER_WEAK = 0x00,
};

/**
 * @brief Internal function to allocate a widget.
 *
 * @param type A widget type.
 * @param widget_class A widget class.
 * @param payload_size A widet payload size.
 *
 * @return Newly allocated and initialized widget.
 */
gp_widget *gp_widget_new(enum gp_widget_type type,
                         enum gp_widget_class widget_class,
                         size_t payload_size);

/**
 * @brief Asserts a non NULL widget.
 *
 * Prints a warning and exits current function when widget is NULL.
 *
 * @param self A widget.
 * @param ret Value to be returned when assertion fails.
 */
#define GP_WIDGET_ASSERT(self, ret) do { \
		if (!self) {\
			GP_BUG("NULL widget!"); \
			return ret; \
		} \
	} while (0)

/**
 * @brief Asserts a widget class.
 *
 * Prints a warning and exits current function when widget class does not match
 * expected class.
 *
 * @param self A widget.
 * @param wclass A widget class, enum #gp_widget_class.
 * @param ret Value to be returned when assertion fails.
 */
#define GP_WIDGET_CLASS_ASSERT(self, wclass, ret) do { \
                GP_WIDGET_ASSERT(self, ret); \
		if (self->widget_class != wclass) { \
			GP_BUG("Invalid widget (%p) class %u != %u", \
			       self, self->widget_class, wclass); \
			return ret; \
		} \
	} while (0)

/**
 * @brief Asserts a widget type.
 *
 * Prints a warning and exits current function when widget type does not match
 * expected type.
 *
 * @param self A widget.
 * @param wtype A widget type, enum #gp_widget_type.
 * @param ret Value to be returned when assertion fails.
 */
#define GP_WIDGET_TYPE_ASSERT(self, wtype, ret) do { \
		GP_WIDGET_ASSERT(self, ret); \
		if (self->type != wtype) {\
			GP_BUG("Invalid widget type %s != %s", \
				gp_widget_type_id(self), gp_widget_type_name(wtype)); \
			return ret; \
		} \
	} while (0)

/**
 * @brief Frees widget memory.
 *
 * Following actions are done when widget is being freed:
 *
 *  - if widget has gp_widget::on_event() event handler set,
 *    #GP_WIDGET_EVENT_FREE event is send
 *  - if widget has children gp_widget_free() is called recursively for all
 *    children widgets
 *  - if widget type defines gp_widget_ops::free() it's called
 *  - widget memory is finally freed
 *
 * Example user widget free event handling
 * ---------------------------------------
 *
 * @code
 * static int event_handler(gp_widget_event *ev)
 * {
 *	if (ev->type == GP_WIDGET_FREE) {
 *		free(ev->self->priv);
 *		return 0;
 *	}
 *
 *	...
 * }
 *
 * int main(void)
 * {
 *	...
 *
 *	void *data = malloc(...);
 *
 *	...
 *
 *	gp_widget_on_event_set(widget, event_handler, data);
 *
 *	...
 *
 *	// the event handler calls free on data from this function
 *	gp_widget_free(widget);
 *
 *	...
 * }
 * @endcode
 *
 * @param self A widget.
 */
void gp_widget_free(gp_widget *self);

/**
 * @brief Sets widget parent.
 *
 * @param self A widget.
 * @param parent A parent widget.
 */
void gp_widget_set_parent(gp_widget *self, gp_widget *parent);

/**
 * @brief Sets focus to a particular widget.
 *
 * Traverses the widget layout tree to the top and sets the focus accordingly.
 *
 * @param self A widget to be focused.
 * @return Zero if focus couldn't be changed, non-zero otherwise.
 */
int gp_widget_focus_set(gp_widget *self);

#include <widgets/gp_widget_event.h>

#endif /* GP_WIDGET_H */
