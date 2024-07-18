//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widget.h>
#include <widgets/gp_widget_ops.h>
#include <widgets/gp_widget_class_bool.h>

bool gp_widget_bool_get(gp_widget *self)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_BOOL, 0);
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);

	return b->val;
}

void gp_widget_bool_set(gp_widget *self, bool val)
{
	GP_WIDGET_CLASS_ASSERT(self, GP_WIDGET_CLASS_BOOL, );
	gp_widget_class_bool *b = GP_WIDGET_CLASS_BOOL(self);

	if (b->val == val)
		return;

	b->val = val;

	gp_widget_redraw(self);
	gp_widget_send_widget_event(self, 0);
}
