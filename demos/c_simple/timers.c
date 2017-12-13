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

/*

  Simple example how to use raw timer priority queue.

 */

#include <gfxprim.h>

uint32_t callback1()
{
	return 0;
}

uint32_t callback3()
{
	return random() % 30 + 1;
}

#define MAX 10

int main(void)
{
	GP_TIMER_DECLARE(oneshot, 30, 0, "Oneshot", callback1, NULL);
	GP_TIMER_DECLARE(recurrent, 0, 4, "Recurrent", callback1, NULL);
	GP_TIMER_DECLARE(random, 10, 0, "Random", callback3, NULL);
	gp_timer timers[MAX];
	gp_timer *queue = NULL;
	uint64_t now;
	int i, ret;
	char ids[MAX][8];

	gp_set_debug_level(10);

	gp_timer_queue_insert(&queue, 0, &oneshot);
	gp_timer_queue_insert(&queue, 0, &recurrent);
	gp_timer_queue_insert(&queue, 0, &random);

	for (i = 0; i < MAX; i++) {
		timers[i].expires = MAX - i;
		timers[i].period = 0;
		timers[i].callback = callback1;
		timers[i].priv = NULL;
		sprintf(ids[i], "Timer%i", MAX - i);
		timers[i].id = ids[i];
		gp_timer_queue_insert(&queue, 0, &timers[i]);
	}

	gp_timer_queue_dump(queue);

	gp_timer_queue_remove(&queue, &timers[MAX-1]);

	gp_timer_queue_dump(queue);

	for (now = 0; now < 100; now += 3) {
		printf("NOW %u\n", (unsigned int) now);
		printf("-------------------------------------\n");
		ret = gp_timer_queue_process(&queue, now);
		printf("Processed %i timer events\n", ret);
		printf("--------------------------------------\n");
		gp_timer_queue_dump(queue);
		printf("--------------------------------------\n\n");
	}

	return 0;
}
