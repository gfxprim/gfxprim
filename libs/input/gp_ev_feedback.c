// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <core/gp_debug.h>
#include <input/gp_ev_feedback.h>

void gp_ev_feedback_set_all(gp_ev_feedback *root, gp_ev_feedback_op *op)
{
	gp_ev_feedback *i;

	GP_DEBUG(2, "Setting 0x%02x leds %s", op->val, op->op == GP_EV_LEDS_ON ? "on" : "off");

	for (i = root; i; i = i->next)
		i->set_get(i, op);
}

void gp_ev_feedback_register(gp_ev_feedback **root, gp_ev_feedback *self)
{
	self->next = *root;
	*root = self;
}

void gp_ev_feedback_unregister(gp_ev_feedback **root, gp_ev_feedback *self)
{
	gp_ev_feedback *i, *prev = NULL;

	for (i = *root; i; i = i->next) {
		if (i == self)
			break;

		prev = i;
	}

	if (!i)
		return;

	if (!prev) {
		*root = (*root)->next;
		return;
	}

	prev->next = i->next;
}
