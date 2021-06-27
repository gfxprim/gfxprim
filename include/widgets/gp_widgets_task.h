//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGETS_TASK_H
#define GP_WIDGETS_TASK_H

#include <input/gp_task.h>

/**
 * @brief Inserts a task into the widgets main loop.
 *
 * @task A pointer to a gp_task.
 */
void gp_widgets_task_ins(gp_task *task);

/**
 * @brief Removes a task from the widgets main loop.
 *
 * @task Pointer to a gp_task.
 */
void gp_widgets_task_rem(gp_task *task);

#endif /* GP_WIDGETS_TASK_H */
