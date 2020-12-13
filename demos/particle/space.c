// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <time.h>
#include "space.h"

struct space *space_create(unsigned int particle_count, int min_w, int min_h,
                           int max_w, int max_h)
{
	struct space *new = malloc(sizeof(struct space) +
	                           sizeof(struct particle) * particle_count);

	if (new == NULL)
		return NULL;

	new->particle_count = particle_count;
	new->min_w = min_w;
	new->min_h = min_h;
	new->max_w = max_w;
	new->max_h = max_h;

	new->gax = 0;
	new->gay = 0;

	new->elasticity = (1<<8) - (1<<6);
	new->mass_kappa = 1<<1;

	unsigned int i;

	srandom(time(NULL));

	for (i = 0; i < particle_count; i++) {
		new->particles[i].x = random() % (max_w - min_w) + min_w;
		new->particles[i].y = random() % (max_h - min_h) + min_h;
		new->particles[i].vx = random() % 40 - 20;
		new->particles[i].vy = random() % 40 - 20;
//		new->particles[i].vx = 0;
//		new->particles[i].vy = 0;
	}

	return new;
}

void space_destroy(struct space *space)
{
	free(space);
}

#define SQUARE(x) ((x) * (x))

void space_draw_particles(gp_pixmap *pixmap, struct space *space)
{
	unsigned int i;

	gp_fill(pixmap, 0x000000);

	for (i = 0; i < space->particle_count; i++) {
		gp_pixel color;

		gp_coord x = space->particles[i].x;
		gp_coord y = space->particles[i].y;

		color = gp_rgb_to_pixmap_pixel(0xee, 0xee, 0xee, pixmap);

		gp_putpixel(pixmap, x>>8, y>>8, color);

		int val = SQUARE(space->particles[i].vx) + SQUARE(space->particles[i].vy);

		val = sqrt(val) + 0x40;

		if (val > 255)
			val = 255;

		color = gp_rgb_to_pixmap_pixel(val, val, 0x40, pixmap);

		gp_circle(pixmap, x>>8, y>>8, 3, color);
	}
}

static void central_gravity(struct space *space, int time)
{
	unsigned int i;

	for (i = 0; i < space->particle_count; i++) {
		space->particles[i].vx += space->gax * time;
		space->particles[i].vy += space->gay * time;
	}
}

#define DIST_X(space, i, j) ((space)->particles[i].x - (space)->particles[j].x)
#define DIST_Y(space, i, j) ((space)->particles[i].y - (space)->particles[j].y)
#define SIGN(x) ((x) < 0 ? -1 : 1)

static void gravity_forces(struct space *space, int time)
{
	unsigned int i, j;

	for (i = 0; i < space->particle_count; i++)
		for (j = 0; j < space->particle_count; j++) {
			int dist_x = DIST_X(space, i, j);
			int dist_y = DIST_Y(space, i, j);

			int dist_squared = (SQUARE((dist_x + (1<<7))>>8) + SQUARE((dist_y + (1<<7))>>8)) + (1<<8);
			int dist = ((int)sqrt(dist_squared))<<4;

			if (dist < (2<<8))
				dist = -(dist>>1);
			else if (dist < (8<<8))
				dist = dist>>1;


			int a = GP_FP_DIV(space->mass_kappa, dist_squared) * time;

			space->particles[i].vx -= (a * dist_x) / dist;
			space->particles[i].vy -= (a * dist_y) / dist;
		}
}

void space_time_tick(struct space *space, int time)
{
	unsigned int i;

	central_gravity(space, time);
	gravity_forces(space, time);

	for (i = 0; i < space->particle_count; i++) {

		if ((space->particles[i].x < space->min_w && space->particles[i].vx < 0) ||
		    (space->particles[i].x >= space->max_w && space->particles[i].vx > 0))
			space->particles[i].vx = GP_FP_MUL(space->particles[i].vx, -space->elasticity);

		if ((space->particles[i].y < space->min_h && space->particles[i].vy < 0) ||
		    (space->particles[i].y >= space->max_h && space->particles[i].vy > 0))
			space->particles[i].vy = GP_FP_MUL(space->particles[i].vy, -space->elasticity);

		space->particles[i].x += space->particles[i].vx * time;
		space->particles[i].y += space->particles[i].vy * time;
	}
}
