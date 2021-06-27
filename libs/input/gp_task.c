// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <input/gp_task.h>

void gp_task_queue_dump(gp_dlist *queue)
{
	gp_dlist_head *i;

	gp_list_foreach(queue, i) {
		gp_task *t = gp_list_entry(i, gp_task, head);

		printf("Task '%s' prio %i callback %p priv %p\n",
		       t->id, t->prio, t->callback, t->priv);
	}
}

void gp_task_queue_ins(gp_dlist *queue, gp_task *task)
{
	gp_dlist_head *i;

	GP_DEBUG(3, "Inserting task '%s' prio %i", task->id, task->prio);

	gp_list_foreach(queue, i) {
		gp_task *t = gp_list_entry(i, gp_task, head);

		if (t->prio > task->prio)
			break;
	}

	if (i)
		gp_dlist_push_before(queue, i, &task->head);
	else
		gp_dlist_push_tail(queue, &task->head);
}

void gp_task_queue_rem(gp_dlist *queue, gp_task *task)
{
	GP_DEBUG(3, "Removing task '%s' prio %i", task->id, task->prio);

	gp_dlist_rem(queue, &task->head);
}

int gp_task_queue_process(gp_dlist *queue)
{
	if (!queue->head)
		return 0;

	gp_task *task = gp_list_entry(queue->head, gp_task, head);

	GP_DEBUG(3, "Running task '%s' prio %i", task->id, task->prio);

	int ret = task->callback(task);

	if (!ret)
		gp_dlist_pop_head(queue);

	return 1;
}
