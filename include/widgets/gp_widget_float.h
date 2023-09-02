//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_FLOAT_H
#define GP_WIDGET_FLOAT_H

struct gp_widget_float {
	double min;
	double max;
	double val;

	int alert:1;

	char payload[];
};

static inline double gp_widget_float_val_get(gp_widget *self)
{
	return self->f->val;
}

void gp_widget_float_set(gp_widget *self, double min, double max, double val);

void gp_widget_float_val_set(gp_widget *self, double val);

void gp_widget_float_max_set(gp_widget *self, double max);

void gp_widget_float_min_set(gp_widget *self, double min);

void gp_widget_float_set_range(gp_widget *self, double min, double max);

#endif /* GP_WIDGET_FLOAT_H */
