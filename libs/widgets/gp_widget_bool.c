//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widget.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_bool.h>

int gp_widget_class_bool_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_BOOL, 0);

	return self->b->val;
}

void gp_widget_class_bool_set(gp_widget *self, int val)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_BOOL, );

	if (self->b->val == val)
		return;

	self->b->val = val;
	gp_widget_redraw(self);
}
