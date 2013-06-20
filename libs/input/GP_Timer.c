/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <inttypes.h>
#include <string.h>

#include "core/GP_Debug.h"
#include "core/GP_Common.h"

#include "input/GP_Timer.h"

static void dump_level(GP_Timer *heap, unsigned int level, unsigned int cur)
{
	if (level == cur) {
		if (heap)
			printf("[%9s %8"PRIu64"] ", heap->id, heap->expires);
		else
			printf("                    ");
		return;
	}

	dump_level(heap ? heap->left : NULL, level, cur+1);
	dump_level(heap ? heap->right : NULL , level, cur+1);
}

void GP_TimerQueueDump(GP_Timer *heap)
{
	unsigned int i, j = 0;

	if (heap == NULL)
		return;

	for (i = 1; heap->sons + 1 >= i; i = i*2) {
		dump_level(heap, j++, 0);
		printf("\n");
	}
}

static int timer_cmp(GP_Timer *t1, GP_Timer *t2)
{
	return t1->expires > t2->expires;
}

/*
 * Returns true if subtree is well balanced i.e. we have 2^n - 2 sons
 */
static int well_balanced(unsigned int sons)
{
	switch (sons) {
	case 0:
	case 2:
	case 6:
	case 15:
	case 30:
	case 62:
	case 126:
	case 254:
	case 510:
	case 1022:
	case 2046:
	case 4092:
	case 8190:
	case 16382:
	case 32766:
		return 1;
	default:
		return 0;
	}
}

static GP_Timer *swap_left(GP_Timer *heap)
{
	GP_Timer *left = heap->left;

	heap->left = left->left;
	left->left = heap;
	GP_SWAP(heap->right, left->right);
	GP_SWAP(heap->sons, left->sons);

	return left;
}

static GP_Timer *swap_right(GP_Timer *heap)
{
	GP_Timer *right = heap->right;			

	heap->right = right->right;
	right->right = heap;
	GP_SWAP(heap->left, right->left);
	GP_SWAP(heap->sons, right->sons);

	return right;
}

/*
 * Inserts timer into binary heap. Returns new root for the tree.
 */
static GP_Timer *insert(GP_Timer *heap, GP_Timer *timer)
{
	if (heap == NULL) {
		timer->left = NULL;
		timer->right = NULL;
		timer->sons = 0;
		return timer;
	}

	heap->sons++;

	if (!heap->left || !well_balanced(heap->left->sons) ||
	     (heap->right && heap->left->sons == heap->right->sons)) {
		
		heap->left = insert(heap->left, timer);
		
		if (timer_cmp(heap, heap->left))
			return swap_left(heap);
	} else {
		
		heap->right = insert(heap->right, timer);

		if (timer_cmp(heap, heap->right))
			return swap_right(heap);
	}

	return heap;
}

static GP_Timer *rem_last(GP_Timer *heap, GP_Timer **last)
{
	if (!heap->left) {
		*last = heap;
		return NULL;
	}

	if (!well_balanced(heap->left->sons) ||
	    !heap->right || (heap->right->sons < heap->left->sons/2))
		heap->left = rem_last(heap->left, last);
	else
		heap->right = rem_last(heap->right, last);

	heap->sons--;

	return heap;
}

static GP_Timer *buble_down(GP_Timer *heap)
{
	GP_Timer *right = heap->right;
	GP_Timer *left = heap->left;

	/* Make sure we choose smaller one */
	if (right && left && timer_cmp(right, left))
		right = NULL;

	if (right && timer_cmp(heap, right)) {
		swap_right(heap);
		right->right = buble_down(heap);
		return right;
	}

	if (left && timer_cmp(heap, left)) {
		swap_left(heap);
		left->left = buble_down(heap);
		return left;
	}

	return heap;
}

static GP_Timer *pop(GP_Timer *heap)
{
	GP_Timer *last;

	if (heap == NULL)
		return NULL;

	heap = rem_last(heap, &last);

	if (!heap)
		return NULL;

	last->left = heap->left;
	last->right = heap->right;
	last->sons = heap->sons;

	return buble_down(last);
}

void GP_TimerQueueInsert(GP_Timer **heap, uint64_t now, GP_Timer *timer)
{
	uint32_t after = timer->period ? timer->period : timer->expires;
	uint64_t expires = now + after;

	GP_DEBUG(3, "Inserting timer %s (now is %"PRIu64") expires after %"
	         PRIu32" at %"PRIu64, timer->id, now, after, expires);

	timer->expires = expires;

	*heap = insert(*heap, timer);
}

static GP_Timer *process_top(GP_Timer *heap, uint64_t now)
{
	GP_Timer *timer = heap;
	uint32_t ret, period;

	GP_DEBUG(3, "Timer %s expired at %"PRIu64" now is %"PRIu64,
	         timer->id, timer->expires, now);

	heap = pop(heap);

	period = timer->period;
	ret = timer->Callback(timer);

	if (period)
		ret = period;

	if (ret) {
		timer->expires = now + ret;
		GP_DEBUG(3, "Rescheduling %stimer %s (now is %"PRIu64") after %"
		         PRIu32" expires at %"PRIu64,
		         period ? "periodic " : "",
		         timer->id, now, ret, timer->expires);
		heap = insert(heap, timer);		
	}

	return heap;
}

int GP_TimerQueueProcess(GP_Timer **heap, uint64_t now)
{
	int ret = 0;

	for (;;) {
		if (*heap == NULL)
			return ret;

		if ((*heap)->expires <= now) {
			*heap = process_top(*heap, now);
			ret++;
		} else {
			return ret;
		}
	}
}
