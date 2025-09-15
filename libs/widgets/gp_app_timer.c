//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2025 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_timer.h>
#include <input/gp_time_stamp.h>
#include <widgets/gp_app_timer.h>

static gp_timer *temp_queue;
static gp_timer **queue = &temp_queue;

void gp_app_timer_start(gp_timer *timer)
{
	gp_timer_queue_ins(queue, gp_time_stamp(), timer);
}

void gp_app_timer_stop(gp_timer *timer)
{
	gp_timer_queue_rem(queue, timer);
}

void gp_app_timer_reschedule(gp_timer *timer, uint32_t expires_ms)
{
        if (gp_timer_is_running(timer))
                gp_timer_queue_rem(queue, timer);

        timer->expires = expires_ms;

        gp_timer_queue_ins(queue, gp_time_stamp(), timer);
}

void gp_app_timer_queue_switch(gp_timer **new_queue)
{
	queue = new_queue;
	*queue = temp_queue;
}
