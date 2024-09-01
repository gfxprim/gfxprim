//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHOICE_PRIV_H
#define GP_WIDGET_CHOICE_PRIV_H

void gp_widget_choice_sel_set_(gp_widget *self, size_t sel) __attribute__((visibility ("hidden")));

static inline size_t call_get_sel(gp_widget *self)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	return choice->ops->get(self, GP_WIDGET_CHOICE_OP_SEL);
}

static inline size_t call_get_cnt(gp_widget *self)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	return choice->ops->get(self, GP_WIDGET_CHOICE_OP_CNT);
}

static inline const char *call_get_choice(gp_widget *self, size_t idx)
{
	struct gp_widget_choice *choice = GP_WIDGET_PAYLOAD(self);

	return choice->ops->get_choice(self, idx);
}

#endif /* GP_WIDGET_CHOICE_PRIV_H */
