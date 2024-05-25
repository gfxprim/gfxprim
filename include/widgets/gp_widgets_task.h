//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widgets_task.h
 * @brief Runs a task in the context of the app main loop.
 *
 * TODO: Rename to gp_app_task()
 */

#ifndef GP_WIDGETS_TASK_H
#define GP_WIDGETS_TASK_H

#include <input/gp_task.h>

/**
 * @brief Inserts a task into the widgets main loop.
 *
 * @note Tasks are not run until the widgets main loop runs.
 *
 * @param task A pointer to a gp_task.
 */
void gp_widgets_task_ins(gp_task *task);

/**
 * @brief Removes a task from the widgets main loop.
 *
 * @note Tasks are not run until the widgets main loop runs.
 *
 * @param task Pointer to a gp_task.
 */
void gp_widgets_task_rem(gp_task *task);

#endif /* GP_WIDGETS_TASK_H */
