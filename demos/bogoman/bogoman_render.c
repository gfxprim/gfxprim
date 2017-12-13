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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <gfxprim.h>

#include "bogoman_map.h"
#include "bogoman_debug.h"

#include "bogoman_render.h"

struct render_colors {
	/* global background */
	gp_pixel bg;

	/* player color */
	gp_pixel player;

	/* frames around things */
	gp_pixel frames;

	/* diamod color */
	gp_pixel diamond;

	/* wall color */
	gp_pixel wall;

	/* moveable color */
	gp_pixel moveable;

	/* edible color */
	gp_pixel edible;

	/* particle colors */
	gp_pixel particle;
	gp_pixel particle_dir;
};

static struct render_colors colors;

static void init_colors(gp_pixmap *pixmap, struct render_colors *colors)
{
	colors->bg           = gp_rgb_to_pixmap_pixel(0xee, 0xee, 0xee, pixmap);
	colors->player       = gp_rgb_to_pixmap_pixel(0x00, 0xee, 0x00, pixmap);
	colors->frames       = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, pixmap);
	colors->diamond      = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0xee, pixmap);
	colors->wall         = gp_rgb_to_pixmap_pixel(0x66, 0x66, 0x66, pixmap);
	colors->moveable     = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0x60, pixmap);
	colors->edible       = gp_rgb_to_pixmap_pixel(0xff, 0x7f, 0x50, pixmap);
	colors->particle     = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0x00, pixmap);
	colors->particle_dir = gp_rgb_to_pixmap_pixel(0xff, 0x44, 0x00, pixmap);
}

static void render_none(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);
}

static void render_player(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);

	gp_fill_circle(render->pixmap, x + w/2, y + w/2, w/2 - 1, colors.player);
	gp_fill_ring(render->pixmap, x + w/2, y + w/2, w/2 - 1, w/2 - 2, colors.frames);
}

static void render_wall(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.wall);

	if (!(elem->flags & BOGOMAN_LEFT)) {
		gp_vline_xyh(render->pixmap, x, y, w, colors.frames);
		gp_vline_xyh(render->pixmap, x+1, y, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_RIGHT)) {
		gp_vline_xyh(render->pixmap, x + w - 1, y, w, colors.frames);
		gp_vline_xyh(render->pixmap, x + w - 2, y, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_UP)) {
		gp_hline_xyw(render->pixmap, x, y, w, colors.frames);
		gp_hline_xyw(render->pixmap, x, y+1, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_DOWN)) {
		gp_hline_xyw(render->pixmap, x, y + w - 1, w, colors.frames);
		gp_hline_xyw(render->pixmap, x, y + w - 2, w, colors.frames);
	}
}

static void render_diamond(struct bogoman_render *render,
                           unsigned int x, unsigned int y,
                           struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);

	(void) elem;

	gp_fill_tetragon(render->pixmap, x + w/2, y, x + w - 1, y + w/2,
	                x + w/2, y + w - 1, x, y + w/2, colors.diamond);

	gp_tetragon(render->pixmap, x + w/2, y, x + w - 1, y + w/2,
	            x + w/2, y + w - 1, x, y + w/2, colors.frames);
	gp_tetragon(render->pixmap, x + w/2, y+1, x + w - 2, y + w/2,
	            x + w/2, y + w - 2, x+1, y + w/2, colors.frames);
}

static void render_moveable(struct bogoman_render *render,
                            unsigned int x, unsigned int y,
                            struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);

	gp_fill_rect_xywh(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.moveable);
	gp_rect_xywh(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.frames);
	gp_rect_xywh(render->pixmap, x + 2, y + 2, w - 4, w - 4, colors.frames);
}

static void render_edible(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);

	gp_fill_rect_xywh(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.edible);
}

static void render_particle(struct bogoman_render *render,
                            unsigned int x, unsigned int y,
                            struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;
	int dir = elem->flags & BOGOMAN_DIRECTION_MASK;

	gp_fill_rect_xywh(render->pixmap, x, y, w, w, colors.bg);

	switch (elem->flags & ~BOGOMAN_DIRECTION_MASK) {
	case BOGOMAN_PARTICLE_ROUND:
		gp_fill_circle(render->pixmap, x + w/2, y + w/2, w/2-1, colors.particle);
		gp_fill_ring(render->pixmap, x + w/2, y + w/2, w/2 - 1, w/2 - 2, colors.frames);
	break;
	case BOGOMAN_PARTICLE_SQUARE:
		gp_fill_rect_xywh(render->pixmap, x+1, y+1, w-2, w-2, colors.particle);
		gp_rect_xywh(render->pixmap, x+1, y+1, w-2, w-2, colors.frames);
		gp_rect_xywh(render->pixmap, x+2, y+2, w-4, w-4, colors.frames);
	break;
	}

	switch (dir) {
	case BOGOMAN_LEFT:
		gp_fill_triangle(render->pixmap, x + w/4, y + w/2,
		                x + 5*w/8, y + w/4, x + 5*w/8, y + 3*w/4, colors.particle_dir);
		gp_triangle(render->pixmap, x + w/4, y + w/2,
		            x + 5*w/8, y + w/4, x + 5*w/8, y + 3*w/4, colors.frames);
	break;
	case BOGOMAN_RIGHT:
		gp_fill_triangle(render->pixmap, x + 3*w/4, y + w/2,
		                x + 3*w/8, y + w/4, x + 3*w/8, y + 3*w/4, colors.particle_dir);
		gp_triangle(render->pixmap, x + 3*w/4, y + w/2,
		            x + 3*w/8, y + w/4, x + 3*w/8, y + 3*w/4, colors.frames);
	break;
	case BOGOMAN_UP:
		gp_fill_triangle(render->pixmap, x + w/2, y + w/4,
		                x + w/4, y + 5*w/8, x + 3*w/4, y + 5*w/8, colors.particle_dir);
		gp_triangle(render->pixmap, x + w/2, y + w/4,
		            x + w/4, y + 5*w/8, x + 3*w/4, y + 5*w/8, colors.frames);
	break;
	case BOGOMAN_DOWN:
		gp_fill_triangle(render->pixmap, x + w/2, y + 3*w/4,
		                x + w/4, y + 3*w/8, x + 3*w/4, y + 3*w/8, colors.particle_dir);
		gp_triangle(render->pixmap, x + w/2, y + 3*w/4,
		            x + w/4, y + 3*w/8, x + 3*w/4, y + 3*w/8, colors.frames);
	break;
	}
}

static void (*renders[])(struct bogoman_render *render,
                       unsigned int x, unsigned int y,
		       struct bogoman_map_elem *elem) =
{
	render_none,
	render_player,
	render_wall,
	render_diamond,
	render_moveable,
	render_edible,
	render_particle,
};

static void render_elem(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			int flags)
{
	struct bogoman_map_elem *elem;
	unsigned int cx, cy;

	elem = bogoman_get_map_elem(render->map, x, y);

	if (elem->dirty) {
		elem->dirty = 0;
	} else {
		if (flags & BOGOMAN_RENDER_DIRTY)
			return;
	}

	cx = (x - render->map_x_offset) * render->map_elem_size;
	cy = (y - render->map_y_offset) * render->map_elem_size;

	if (elem->id > BOGOMAN_MAX)
		WARN("Invalid elem ID %u at %ux%u\n", elem->id, x, y);
	else
		renders[elem->id](render, cx, cy, elem);

	if (flags & BOGOMAN_RENDER_DIRTY && render->backend) {
		gp_backend_update_rect(render->backend, cx, cy,
		                     cx + render->map_elem_size,
		                     cy + render->map_elem_size);
	}
}

void bogoman_render(struct bogoman_render *render, int flags)
{
	unsigned int x, y;

	//TODO: Hack
	init_colors(render->pixmap, &colors);

	if (flags & BOGOMAN_RENDER_ALL)
		gp_fill(render->pixmap, colors.bg);

	for (y = render->map_x_offset; y < render->map->h; y++) {
		for (x = render->map_x_offset; x < render->map->w; x++)
			render_elem(render, x, y, flags);
	}

	if (flags & BOGOMAN_RENDER_ALL && render->backend)
		gp_backend_flip(render->backend);
}
