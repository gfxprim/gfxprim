// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Simple example how to use raw timer queue.

 */

#include <gfxprim.h>

uint32_t callback_oneshot(gp_timer *self)
{
	(void) self;

	return GP_TIMER_STOP;
}

static uint32_t callback_periodic(gp_timer *self)
{
	return self->period;
}

uint32_t callback_random(gp_timer *self)
{
	(void) self;

	return random() % 30 + 1;
}

#define MAX 10

int main(void)
{
	GP_TIMER_DECLARE(oneshot, 30, 0, "Oneshot", callback_oneshot, NULL);
	GP_TIMER_DECLARE(recurrent, 0, 4, "Recurrent", callback_periodic, NULL);
	GP_TIMER_DECLARE(random, 10, 0, "Random", callback_random, NULL);
	gp_timer timers[MAX];
	gp_timer_queue queue = {};
	uint64_t now;
	int i, ret;
	char ids[MAX][8];

	gp_set_debug_level(10);

	gp_timer_queue_ins(&queue, 0, &oneshot);
	gp_timer_queue_ins(&queue, 0, &recurrent);
	gp_timer_queue_ins(&queue, 0, &random);

	for (i = 0; i < MAX; i++) {
		sprintf(ids[i], "Timer%i", MAX-i);
		gp_timer_init(&timers[i], MAX-i, 0, ids[i], callback_oneshot, NULL);
		gp_timer_queue_ins(&queue, 0, &timers[i]);
	}

	printf("All timers inserted:\n");
	gp_timer_queue_dump(&queue);

	printf("Timer1 removed:\n");
	gp_timer_queue_rem(&queue, &timers[MAX-1]);
	gp_timer_queue_dump(&queue);

	for (now = 0; now < 100; now += 3) {
		printf("NOW %u\n", (unsigned int) now);
		printf("-------------------------------------\n");
		ret = gp_timer_queue_process(&queue, now);
		printf("Processed %i timer events\n", ret);
		printf("--------------------------------------\n");
		gp_timer_queue_dump(&queue);
		printf("--------------------------------------\n\n");
	}

	return 0;
}
