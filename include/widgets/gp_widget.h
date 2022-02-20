//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_H
#define GP_WIDGET_H

#include <stdlib.h>
#include <stdint.h>
#include <utils/gp_types.h>
#include <widgets/gp_common.h>
#include <widgets/gp_widget_types.h>

struct gp_widget {
	unsigned int type;
	unsigned int widget_class;
	gp_widget *parent;

	/*
	 * Widget event handler.
	 */
	int (*on_event)(gp_widget_event *);

	/*
	 * User provided pointer to arbitrary data; useful for event handlers
	 * and other user defined functions. The library will not access or
	 * modify the memory pointed to by this.
	 */
	void *priv;

	/*
	 * Relative offset to the parent widget.
	 */
	unsigned int x, y;

	/*
	 * Current widget size.
	 */
	unsigned int w, h;

	/*
	 * Cached widget minimal size.
	 */
	unsigned int min_w, min_h;

	unsigned int align:16;
	/*
	 * If set widget will not shrink, i.e. will be resized only when the
	 * new minimal size is bigger than the previous one.
	 *
	 * By default this is set to 0.
	 */
	unsigned int no_shrink:1;

	unsigned int no_resize:1;
	/*
	 * If set the widget_ops_render() is called next time layout is repainted.
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
	unsigned int focused:1;

	/*
	 * If set the widget cannot get focused and get events even if
	 * ops->event is implemented.
	 */
	unsigned int no_events:1;

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

		void *payload;
	};
	char buf[];
};

enum gp_widget_type {
	GP_WIDGET_GRID,
	GP_WIDGET_TABS,
	GP_WIDGET_BUTTON,
	GP_WIDGET_CHECKBOX,
	GP_WIDGET_LABEL,
	GP_WIDGET_PROGRESSBAR,
	GP_WIDGET_SPINNER,
	GP_WIDGET_SLIDER,
	GP_WIDGET_TBOX,
	GP_WIDGET_RADIOBUTTON,
	GP_WIDGET_SPINBUTTON,
	GP_WIDGET_TABLE,
	GP_WIDGET_PIXMAP,
	GP_WIDGET_STOCK,
	GP_WIDGET_SCROLL_AREA,
	GP_WIDGET_FRAME,
	GP_WIDGET_MARKUP,
	GP_WIDGET_SWITCH,
	GP_WIDGET_OVERLAY,
	GP_WIDGET_LOG,
	GP_WIDGET_MAX,
};

enum gp_widget_class {
	GP_WIDGET_CLASS_NONE = 0,
	GP_WIDGET_CLASS_BOOL,
	GP_WIDGET_CLASS_INT,
	GP_WIDGET_CLASS_CHOICE,
	GP_WIDGET_CLASS_MAX,
};

/**
 * @brief Returns widget class name.
 */
const char *gp_widget_class_name(enum gp_widget_class widget_class);

enum gp_widget_alignment {
	/** Default overridable alignment. */
	GP_HCENTER_WEAK = 0x00,
	GP_HCENTER = 0x01,
	GP_LEFT    = 0x02,
	GP_RIGHT   = 0x03,
	GP_HFILL   = 0x08,
	/** Default overridable alignment. */
	GP_VCENTER_WEAK = 0x00,
	GP_VCENTER = 0x10,
	GP_TOP     = 0x20,
	GP_BOTTOM  = 0x30,
	GP_VFILL   = 0x80,
};

#define GP_FILL (GP_VFILL | GP_HFILL)

#define GP_HALIGN_MASK 0x0f
#define GP_VALIGN_MASK 0xf0

/**
 * @brief Internal function to allocate a widget.
 *
 * @type A widget type.
 * @class A widget class.
 * @payload_size A widet payload size.
 *
 * @return Newly allocated and initialized widget.
 */
gp_widget *gp_widget_new(enum gp_widget_type type,
                         enum gp_widget_class widget_class,
                         size_t payload_size);

#define GP_WIDGET_CLASS_ASSERT(self, wclass, ret) do {                  \
		if (!self) {                                            \
			GP_BUG("NULL widget!");                         \
			return ret;                                     \
		}                                                       \
                if (self->widget_class != wclass) {                     \
			GP_BUG("Invalid widget (%p) class %u != %u",    \
			       self, self->widget_class, wclass);       \
			return ret;                                     \
		}                                                       \
	} while (0)

#define GP_WIDGET_ASSERT(self, wtype, ret) do { \
		if (!self) {\
			GP_BUG("NULL widget!"); \
			return ret; \
		} else if (self->type != wtype) {\
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
 *  - if widget has event handler GP_WIDGET_EVENT_FREE is send
 *  - if needed gp_widget_free() is called recursively for all children widgets
 *  - if widget type defines free() in it's ops it's called
 *  - widget memory is finally freed
 *
 * @self A widget.
 */
void gp_widget_free(gp_widget *self);

/**
 * @brief Sets widget parent.
 *
 * @self A widget.
 * @parent A parent widget.
 */
void gp_widget_set_parent(gp_widget *self, gp_widget *parent);

/**
 * @brief Sets focus to a particular widget.
 *
 * Traverses the widget layout tree to the top and sets the focus accordingly.
 *
 * @self A widget to be focused.
 *
 * @return Zero if focus couldn't be changed, non-zero otherwise.
 */
int gp_widget_focus_set(gp_widget *self);


/**
 * @brief Sets widget event handler.
 *
 * Note that even after setting event handler certain widget events has to be
 * unmasked in order to receive them.
 *
 * @self A widget.
 * @on_event An widget event handler.
 * @priv An user pointer stored in the widget.
 */
static inline void gp_widget_on_event_set(gp_widget *self,
                                          int (*on_event)(gp_widget_event *),
                                          void *priv)
{
	self->on_event = on_event;
	self->priv = priv;
	self->no_events = 0;
}

#include <widgets/gp_widget_event.h>

#endif /* GP_WIDGET_H */
