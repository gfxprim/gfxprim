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

 /*

   Particle demo.

  */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <GP.h>

struct particle {
	/* fixed point coordinates */
	int x;
	int y;

	/* fixed point speed */
	int vx;
	int vy;
};

struct space {
	unsigned int particle_count;

	/* space is an rectanle */
	int min_w;
	int min_h;
	
	int max_w;
	int max_h;

	/* gravitation vector */
	int gax;
	int gay;

	/* elasticity at the boudaries */
	int elasticity;

	/* particle mass */
	int mass_kappa;

	struct particle particles[];
};

struct space *space_create(unsigned int particle_count, int min_w, int min_h,
                           int max_w, int max_h);

void space_destroy(struct space *space);

void space_draw_particles(GP_Context *context, struct space *space);

void space_time_tick(struct space *space, int time);

#endif /* PARTICLE_H */
