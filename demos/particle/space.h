// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Particle demo.

  */

#ifndef PARTICLE_H
#define PARTICLE_H

#include <gfxprim.h>

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

void space_draw_particles(gp_pixmap *pixmap, struct space *space);

void space_time_tick(struct space *space, int time);

#endif /* PARTICLE_H */
