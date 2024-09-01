//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

#include <widgets/gp_widgets.h>

#include "gp_widget_grid_priv.h"

#define ASSERT_HVBOX(g, ret) \
	do { \
		if (g->cols != 1 && g->rows != 1) { \
			GP_BUG("Grid widget is not a hbox or vbox!"); \
			return ret; \
		} \
	} while (0)

gp_widget *gp_widget_box_put(gp_widget *self, unsigned int pos, gp_widget *child)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	ASSERT_HVBOX(grid, NULL);

	if (grid->cols == 1)
		return gp_widget_grid_put(self, 0, pos, child);
	else
		return gp_widget_grid_put(self, pos, 0, child);
}

gp_widget *gp_widget_box_rem(gp_widget *self, unsigned int pos)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_GRID, NULL);
	struct gp_widget_grid *grid = GP_WIDGET_PAYLOAD(self);
	ASSERT_HVBOX(grid, NULL);

	if (grid->cols == 1)
		return gp_widget_grid_rem(self, 0, pos);
	else
		return gp_widget_grid_rem(self, pos, 0);
}
