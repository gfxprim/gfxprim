//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_INT_H
#define GP_WIDGET_INT_H

enum gp_widget_int_flags {
	GP_WIDGET_HORIZ = 0,
	GP_WIDGET_VERT = 1
};

struct gp_widget_int {
	int64_t min;
	int64_t max;
	int64_t val;

	int alert:1;
	int dir:2;

	char payload[];
};

int64_t gp_widget_int_val_get(gp_widget *self);

void gp_widget_int_set(gp_widget *self, int64_t min, int64_t max, int64_t val);

void gp_widget_int_val_set(gp_widget *self, int64_t val);

void gp_widget_int_max_set(gp_widget *self, int64_t max);

void gp_widget_int_min_set(gp_widget *self, int64_t min);

void gp_widget_int_set_range(gp_widget *self, int64_t min, int64_t max);

#endif /* GP_WIDGET_INT_H */
