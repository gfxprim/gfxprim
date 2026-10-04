//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_json.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_ops.h>

const char *gp_widget_class_name(enum gp_widget_class widget_class)
{
	switch (widget_class) {
	case GP_WIDGET_CLASS_NONE:
		return "none";
	case GP_WIDGET_CLASS_BOOL:
		return "bool";
	case GP_WIDGET_CLASS_INT:
		return "int";
	case GP_WIDGET_CLASS_CHOICE:
		return "choice";
	default:
		return "???";
	}
}

gp_widget *gp_widget_new(enum gp_widget_type type,
                         enum gp_widget_class widget_class,
                         size_t payload_size)
{
	size_t size = sizeof(gp_widget) + payload_size;
	gp_widget *ret = malloc(size);

	GP_DEBUG(1, "Allocating widget %s class %s payload_size=%zu size=%zu",
	         gp_widget_type_name(type), gp_widget_class_name(widget_class),
	         payload_size, size);

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	memset(ret, 0, size);
	ret->type = type;
	ret->widget_class = widget_class;

	ret->event_mask = GP_WIDGET_EVENT_DEFAULT_MASK;

	return ret;
}

void gp_widget_set_parent(gp_widget *self, gp_widget *parent)
{
	if (!self)
		return;

	//TODO: reparent?
	if (self->parent)
		GP_WARN("Widget %p has already parent %p!", self, self->parent);

	self->parent = parent;
}

gp_widget *gp_widget_layout_root(gp_widget *self)
{
	gp_widget *root = self;

	if (!self)
		return NULL;

	while (root->parent)
		root = root->parent;

	GP_DEBUG(3, "Looked up %p root %p", self, root);

	return root;
}

struct widget_dump {
	int level;
	bool (*filter)(gp_widget *self);
};

static void widget_dump(gp_widget *self, struct widget_dump *dump)
{
	int i;

	for (i = 0; i < dump->level; i++)
		printf(" ");

	printf("%p (%12s) redraw=%i redraw_child=%i resized=%i focused=%i disabled=%i %ix%i\n",
	       self, gp_widget_type_name(self->type),
	       self->redraw, self->redraw_child, self->resized,
	       self->focused, self->disabled, self->w, self->h);
}

static void widget_tree_dump(gp_widget *self, void *priv)
{
	struct widget_dump *dump = priv;

	if (dump->filter(self))
		widget_dump(self, dump);

	dump->level++;
	gp_widget_ops_for_each_child_priv(self, widget_tree_dump, priv);
	dump->level--;
}

void gp_widget_layout_dump(gp_widget *self, bool (*filter)(gp_widget *self))
{
	struct widget_dump dump = {.filter = filter};

	if (!self)
		return;

	widget_tree_dump(self, &dump);
}

void gp_widget_free(gp_widget *self)
{
	const struct gp_widget_ops *ops;

	if (!self)
		return;

	gp_widget_send_event(self, GP_WIDGET_EVENT_FREE, 0);

	gp_widget_ops_for_each_child(self, gp_widget_free);

	ops = gp_widget_ops(self);
	if (ops->free)
		ops->free(self);

	free(self);
}

void gp_widget_disable(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, );

	if (self->disabled)
		return;

	self->disabled = 1;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

void gp_widget_enable(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, );

	if (!self->disabled)
		return;

	self->disabled = 0;

	gp_widget_redraw(self);
	gp_widget_redraw_children(self);
}

void gp_widget_disabled_set(gp_widget *self, bool disabled)
{
	if (disabled)
		gp_widget_disable(self);
	else
		gp_widget_enable(self);
}

bool gp_widget_disabled_get(gp_widget *self)
{
	GP_WIDGET_ASSERT(self, false);

	return self->disabled;
}
