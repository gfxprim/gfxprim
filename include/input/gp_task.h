// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Tasks are sorted into queues accordingly to its priorities. Queue with
  smallest priority runs until it's out of tasks. Tasks inside a single queue
  are schedulled by a round robin.

 */

#ifndef INPUT_GP_TASK_H
#define INPUT_GP_TASK_H

#include <input/gp_types.h>
#include <utils/gp_list.h>

#define GP_TASK_NONE_PRIO 0
/* These are just bounds for the priorities. */
#define GP_TASK_MIN_PRIO 1
#define GP_TASK_MAX_PRIO 3

struct gp_task_queue {
	unsigned int task_cnt;
	unsigned int min_prio;
	gp_dlist queues[GP_TASK_MAX_PRIO - GP_TASK_MIN_PRIO + 1];
};

struct gp_task {
	gp_dlist_head head;
	unsigned int prio;
	char *id;
	int (*callback)(gp_task *self);
	void *priv;
};

void gp_task_queue_dump(gp_task_queue *self);

void gp_task_queue_ins(gp_task_queue *self, gp_task *task);

void gp_task_queue_rem(gp_task_queue *self, gp_task *task);

int gp_task_queue_process(gp_task_queue *self);

static inline size_t gp_task_queue_tasks(gp_task_queue *self)
{
	return self->task_cnt;
}

static inline unsigned int gp_task_queue_head_prio(gp_task_queue *self)
{
	return self->min_prio;
}

#endif /* INPUT_GP_TASK_H */
