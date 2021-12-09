//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_INT_H
#define GP_WIDGET_INT_H

enum gp_widget_int_flags {
	GP_WIDGET_HORIZ = 0,
	GP_WIDGET_VERT = 1
};

struct gp_widget_int {
	int min, max, val;
	int alert:1;
	int dir:2;

	char payload[];
};

static inline int gp_widget_int_val_get(gp_widget *self)
{
	return self->i->val;
}

void gp_widget_int_set(gp_widget *self, int min, int max, int val);

void gp_widget_int_val_set(gp_widget *self, int val);

void gp_widget_int_max_set(gp_widget *self, int max);

void gp_widget_int_min_set(gp_widget *self, int min);

#endif /* GP_WIDGET_INT_H */
