//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_timer.h
 * @brief Inserts timer into a main loop.
 *
 *TODO: Rename to app timer.
 */

#ifndef GP_WIDGET_TIMER_H
#define GP_WIDGET_TIMER_H

/**
 * @brief Inserts a timer into the widgets timer queue.
 *
 * @note No timers will expire until the widgets main loop runs.
 *
 * @param timer Pointer to a gp_timer.
 */
void gp_widgets_timer_ins(gp_timer *timer);

/**
 * @brief Removes a timer from the widgets timer queue.
 *
 * @note No timers will expire until the widgets main loop runs.
 *
 * @param timer Pointer to a gp_timer.
 */
void gp_widgets_timer_rem(gp_timer *timer);

#endif /* GP_WIDGET_TIMER_H */
