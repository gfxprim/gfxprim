//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_KEY_REPEAT_TIMER__
#define GP_KEY_REPEAT_TIMER__

#include <input/gp_types.h>

void gp_key_repeat_timer_init(gp_event_queue *ev_queue, gp_timer **tmr_queue);

void gp_handle_key_repeat_timer(gp_event *ev);

#endif /* GP_KEY_REPEAT_TIMER___ */
