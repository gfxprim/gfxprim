// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_task.h
 * @brief Application tasks.
 *
 * Tasks a way how to schedule a short amount of work in the context of the
 * application main loop. Tasks are sorted into queues accordingly to its
 * priorities. Queue with smallest priority runs until it's out of tasks.
 * Tasks inside a single queue are scheduled by a round robin.
 */

#ifndef INPUT_GP_TASK_H
#define INPUT_GP_TASK_H

#include <input/gp_types.h>
#include <utils/gp_list.h>

#define GP_TASK_NONE_PRIO 0
/* These are just bounds for the priorities. */
#define GP_TASK_MIN_PRIO 1
#define GP_TASK_MAX_PRIO 3

/** @brief A task queue. */
struct gp_task_queue {
	/** @brief A number of task in the queue. */
	unsigned int task_cnt;
	/**
	 * @brief A minimal priority for a non-empty queue.
	 *
	 * If queue is empty it's set to GP_TASK_NONE_PRIO.
	 */
	unsigned int min_prio;
	/** @brief A linked list per priority queue. */
	gp_dlist queues[GP_TASK_MAX_PRIO - GP_TASK_MIN_PRIO + 1];
};

/**
 * @brief A task.
 *
 * Task is a callback with some more information such as priority. Tasks are
 * queued in the #gp_task_queue and executed sequentially accordingly to the
 * priorities.
 */
struct gp_task {
	/** @brief A pointers to the linked list. */
	gp_dlist_head head;
	/** @brief A task priority. */
	unsigned int prio:3;
	/** @brief Set if task is queued in the queue. */
	unsigned int queued:1;
	/** @brief Human readable task id. */
	char *id;
	/**
	 * @brief The task callback.
	 *
	 * @param self A task structure.
	 * @return If zero is returned the callback is finished i.e. removed
	 *         from the queue and the queued flag is cleared, otherwise
	 *         it's rescheduled.
	 */
	int (*callback)(gp_task *self);
	/** @brief A private pointer to be used by the user of the API. */
	void *priv;
};

/**
 * @brief Dumps task queue structure into stdout.
 *
 * @param self A task queue.
 */
void gp_task_queue_dump(gp_task_queue *self);

/**
 * @brief Inserts a new task into the queue.
 *
 * @param self A task queue.
 * @param task A task to be inserted.
 */
void gp_task_queue_ins(gp_task_queue *self, gp_task *task);

/**
 * @brief Removes a new task from the queue.
 *
 * @param self A task queue.
 * @param task A task to be removed.
 */
void gp_task_queue_rem(gp_task_queue *self, gp_task *task);

/**
 * @brief Runs a single task in the queue.
 *
 * Runs a single task from first smaller priority the non-empty queue. When
 * task finishes it's work it is either removed from the queue or is requeued
 * based on the return value from the task callback.
 *
 * @param self A task queue.
 *
 * @return A number of tasks processed, i.e. zero if queue is empty, one
 *         otherwise.
 */
int gp_task_queue_process(gp_task_queue *self);


/**
 * @brief Returns number of tasks in the queue.
 *
 * @param self A task queue.
 *
 * @return A number of tasks waiting in the queue.
 */
static inline size_t gp_task_queue_tasks(gp_task_queue *self)
{
	return self->task_cnt;
}

/**
 * @brief Returns minimal priority from all tasks in the queue.
 *
 * @param self A task queue.
 *
 * @return A minimal priority from all tasks in the queue, or GP_TASK_PRIO_NONE
 *         if the queue is empty.
 */
static inline unsigned int gp_task_queue_head_prio(gp_task_queue *self)
{
	return self->min_prio;
}

#endif /* INPUT_GP_TASK_H */
