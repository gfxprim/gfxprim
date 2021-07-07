// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <input/gp_task.h>


static inline gp_dlist *queue_by_prio(gp_task_queue *self, unsigned int prio)
{
	return &self->queues[prio - GP_TASK_MIN_PRIO];
}

void gp_task_queue_dump(gp_task_queue *self)
{
	gp_dlist_head *i;
	unsigned int prio;

	printf("Total tasks %u min_prio %u", self->task_cnt, self->min_prio);

	for (prio = GP_TASK_MIN_PRIO; prio <= GP_TASK_MAX_PRIO; prio++) {
		gp_list_foreach(queue_by_prio(self, prio), i) {
			gp_task *t = gp_list_entry(i, gp_task, head);

			printf("Task '%s' prio %i callback %p priv %p\n",
			       t->id, t->prio, t->callback, t->priv);
		}
	}
}

static inline gp_dlist *queue_by_prio_check(gp_task_queue *self, unsigned int prio)
{
	if (prio < GP_TASK_MIN_PRIO || prio > GP_TASK_MAX_PRIO) {
		GP_WARN("Invalid task priority %u", prio);
		return NULL;
	}

	return &self->queues[prio - GP_TASK_MIN_PRIO];
}

static inline unsigned int find_queue_min_prio(gp_task_queue *self)
{
	unsigned int prio;
	gp_dlist *queue;

	if (self->min_prio == GP_TASK_NONE_PRIO) {
		GP_BUG("Removing from empty task queue!?");
		return GP_TASK_NONE_PRIO;
	}

	queue = queue_by_prio(self, self->min_prio);
	if (queue->cnt)
		return self->min_prio;

	for (prio = GP_TASK_MIN_PRIO; prio <= GP_TASK_MAX_PRIO; prio++) {
		queue = queue_by_prio(self, prio);

		if (queue->cnt)
			return prio;
	}

	return GP_TASK_NONE_PRIO;
}

void gp_task_queue_ins(gp_task_queue *self, gp_task *task)
{
	gp_dlist *queue;

	if (task->queued) {
		GP_DEBUG(1, "Task '%s' prio %u already queued", task->id, task->prio);
		return;
	}

	queue = queue_by_prio_check(self, task->prio);
	if (!queue)
		return;

	GP_DEBUG(3, "Inserting task '%s' prio %u", task->id, task->prio);

	gp_dlist_push_tail(queue, &task->head);

	self->task_cnt++;

	if (self->min_prio == GP_TASK_NONE_PRIO)
		self->min_prio = task->prio;
	else
		self->min_prio = GP_MIN(self->min_prio, (unsigned int)task->prio);

	task->queued = 1;
}

void gp_task_queue_rem(gp_task_queue *self, gp_task *task)
{
	gp_dlist *queue;

	if (!task->queued) {
		GP_DEBUG(1, "Task '%s' prio %u is not queued", task->id, task->prio);
		return;
	}

	queue = queue_by_prio_check(self, task->prio);
	if (!queue)
		return;

	GP_DEBUG(3, "Removing task '%s' prio %i", task->id, task->prio);

	gp_dlist_rem(&self->queues[task->prio], &task->head);

	self->task_cnt--;
	self->min_prio = find_queue_min_prio(self);

	task->queued = 0;
}

int gp_task_queue_process(gp_task_queue *self)
{
	if (self->min_prio == GP_TASK_NONE_PRIO)
		return 0;

	gp_dlist *task_list = queue_by_prio(self, self->min_prio);
	gp_dlist_head *task_head = gp_dlist_pop_head(task_list);
	gp_task *task = gp_list_entry(task_head, gp_task, head);

	GP_DEBUG(3, "Running task '%s' prio %i", task->id, task->prio);

	int ret = task->callback(task);

	if (ret) {
		gp_dlist_push_tail(task_list, task_head);
	} else {
		self->task_cnt--;
		self->min_prio = find_queue_min_prio(self);
		task->queued = 0;
	}

	return 1;
}
