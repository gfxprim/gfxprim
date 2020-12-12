//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <gp_widget.h>
#include <gp_widget_ops.h>

gp_widget *gp_widget_new(enum gp_widget_type type, size_t payload_size)
{
	size_t size = sizeof(gp_widget) + payload_size;
	gp_widget *ret = malloc(size);

	GP_DEBUG(1, "Allocating widget %s payload_size=%zu size=%zu",
	         gp_widget_type_name(type), payload_size, size);

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	memset(ret, 0, size);
	ret->payload = ret->buf;
	ret->type = type;

	ret->event_mask = GP_WIDGET_DEFAULT_EVENT_MASK;

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
