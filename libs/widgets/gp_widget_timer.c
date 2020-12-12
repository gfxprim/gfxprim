//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <input/gp_timer.h>
#include <input/gp_time_stamp.h>
#include <gp_widget_timer.h>

static gp_timer *temp_queue;
static gp_timer **queue = &temp_queue;

void gp_widgets_timer_ins(gp_timer *timer)
{
	gp_timer_queue_insert(queue, gp_time_stamp(), timer);
}

void gp_widgets_timer_rem(gp_timer *timer)
{
	gp_timer_queue_remove(queue, timer);
}

void gp_widget_timer_queue_switch(gp_timer **new_queue)
{
	queue = new_queue;
	*queue = temp_queue;
}
