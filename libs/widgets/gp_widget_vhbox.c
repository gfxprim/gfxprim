//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

#define ASSERT_HVBOX(w, ret) \
	do { \
		if (self->grid->cols != 1 && self->grid->rows != 1) { \
			GP_BUG("Grid widget is not a hbox or vbox!"); \
			return ret; \
		} \
	} while (0)

gp_widget *gp_widget_box_put(gp_widget *self, unsigned int pos, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);
	ASSERT_HVBOX(self, NULL);

	if (self->grid->cols == 1)
		return gp_widget_grid_put(self, 0, pos, child);
	else
		return gp_widget_grid_put(self, pos, 0, child);
}

gp_widget *gp_widget_box_rem(gp_widget *self, unsigned int pos)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);
	ASSERT_HVBOX(self, NULL);

	if (self->grid->cols == 1)
		return gp_widget_grid_rem(self, 0, pos);
	else
		return gp_widget_grid_rem(self, pos, 0);
}
