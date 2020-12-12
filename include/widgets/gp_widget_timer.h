//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TIMER_H__
#define GP_WIDGET_TIMER_H__

/**
 * @brief Inserts a timer into the widgets timer queue.
 *
 * NOTE: No timers will expire until the widgets main loop runs.
 *
 * @timer Pointer to a gp_timer.
 */
void gp_widgets_timer_ins(gp_timer *timer);

/**
 * @brief Removes a timer from the widgets timer queue.
 *
 * NOTE: No timers will expire until the widgets main loop runs.
 *
 * @timer Pointer to a gp_timer.
 */
void gp_widgets_timer_rem(gp_timer *timer);

#endif /* GP_WIDGET_TIMER_H__ */
