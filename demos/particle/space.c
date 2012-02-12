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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "space.h"

struct space *space_create(unsigned int particle_count, int w, int h)
{
	struct space *new = malloc(sizeof(struct space) +
	                           sizeof(struct particle) * particle_count);

	if (new == NULL)
		return NULL;

	new->particle_count = particle_count;
	new->w = w;
	new->h = h;

	unsigned int i;

	for (i = 0; i < particle_count; i++) {
		new->particles[i].x = random() % w;
		new->particles[i].y = random() % h;
		new->particles[i].vx = random() % 40 - 20;
		new->particles[i].vy = random() % 40 - 20;
	}

	return new;
}

void space_destroy(struct space *space)
{
	free(space);
}

void space_draw_particles(GP_Context *context, struct space *space)
{
	unsigned int i;
	
	GP_Fill(context, 0);

	for (i = 0; i < space->particle_count; i++)
		GP_PutPixelAA(context, space->particles[i].x, space->particles[i].y, 0xffffff);
}

static void modify_speeds(struct space *space, int time)
{
	unsigned int i, j;

	for (i = 0; i < space->particle_count; i++) { 
//			space->particles[i].vx +=  * time;
			space->particles[i].vy += time; 
		}
}

void space_time_tick(struct space *space, int time)
{
	unsigned int i;

	modify_speeds(space, time);

	for (i = 0; i < space->particle_count; i++) {
		if (space->particles[i].x <= 2 || space->particles[i].x >= space->w - 2)
			space->particles[i].vx *= -0.9;
		
		if (space->particles[i].y <= 2 || space->particles[i].y >= space->h - 2)
			space->particles[i].vy *= -0.9;
		
		space->particles[i].x += space->particles[i].vx * time;
		space->particles[i].y += space->particles[i].vy * time;
	}
}
