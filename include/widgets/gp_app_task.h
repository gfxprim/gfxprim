//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2025 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_task.h
 * @brief Runs a task in the context of the app main loop.
 */

#ifndef WIDGETS_GP_APP_TASK_H
#define WIDGETS_GP_APP_TASK_H

#include <input/gp_task.h>

/**
 * @brief Inserts a task into the widgets main loop.
 *
 * @note Tasks are not run until the widgets main loop runs.
 *
 * @param task A pointer to a gp_task.
 */
void gp_app_task_start(gp_task *task);

/**
 * @brief Removes a task from the widgets main loop.
 *
 * @note Tasks are not run until the widgets main loop runs.
 *
 * @param task Pointer to a gp_task.
 */
void gp_app_task_stop(gp_task *task);

#endif /* WIDGETS_GP_APP_TASK_H */
