//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2025 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_timer.h
 * @brief Manipulates timers in an application main loop.
 */

#ifndef WIDGETS_GP_APP_TIMER_H
#define WIDGETS_GP_APP_TIMER_H

/**
 * @brief Inserts a timer into the widgets timer queue.
 *
 * @note No timers will expire until the widgets main loop runs.
 *
 * @param timer Pointer to a gp_timer.
 */
void gp_app_timer_start(gp_timer *timer);

/**
 * @brief Removes a timer from the widgets timer queue.
 *
 * @note No timers will expire until the widgets main loop runs.
 *
 * @param timer Pointer to a gp_timer.
 */
void gp_app_timer_stop(gp_timer *timer);

/**
 * @brief Reschedules a timer.
 *
 * If a timer is running (inserted into the timer queue) it's stopped before
 * it's started again.
 *
 * @param timer A timer.
 * @param expires_ms A new timeout for the timer in miliseconds.
 */
void gp_app_timer_reschedule(gp_timer *timer, uint32_t expires_ms);

#endif /* WIDGETS_GP_APP_TIMER_H */
