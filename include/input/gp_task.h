// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Tasks are sorted by a priority smaller prio means that task is executed
  sooner.

 */

#ifndef INPUT_GP_TASK_H
#define INPUT_GP_TASK_H

#include <input/gp_types.h>
#include <utils/gp_list.h>

struct gp_task {
	gp_dlist_head head;
	unsigned int prio;
	char *id;
	int (*callback)(gp_task *self);
	void *priv;
};

void gp_task_queue_dump(gp_dlist *queue);

void gp_task_queue_ins(gp_dlist *queue, gp_task *task);

void gp_task_queue_rem(gp_dlist *queue, gp_task *task);

int gp_task_queue_process(gp_dlist *queue);

static inline size_t gp_task_queue_tasks(gp_dlist *queue)
{
	return queue->cnt;
}

static inline unsigned int gp_task_queue_head_prio(gp_dlist *queue)
{
	gp_task *task = gp_list_entry(queue->head, gp_task, head);

	return task->prio;
}

#endif /* INPUT_GP_TASK_H */
