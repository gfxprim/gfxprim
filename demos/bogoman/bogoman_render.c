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

#include <GP.h>

#include "bogoman_map.h"
#include "bogoman_debug.h"

#include "bogoman_render.h"

struct render_colors {
	/* global background */
	GP_Pixel bg;

	/* player color */
	GP_Pixel player;

	/* frames around things */
	GP_Pixel frames;

	/* diamod color */
	GP_Pixel diamond;

	/* wall color */
	GP_Pixel wall;

	/* moveable color */
	GP_Pixel moveable;

	/* edible color */
	GP_Pixel edible;

	/* particle colors */
	GP_Pixel particle;
	GP_Pixel particle_dir;
};

static struct render_colors colors;

static void init_colors(GP_Pixmap *pixmap, struct render_colors *colors)
{
	colors->bg           = GP_RGBToPixmapPixel(0xee, 0xee, 0xee, pixmap);
	colors->player       = GP_RGBToPixmapPixel(0x00, 0xee, 0x00, pixmap);
	colors->frames       = GP_RGBToPixmapPixel(0x00, 0x00, 0x00, pixmap);
	colors->diamond      = GP_RGBToPixmapPixel(0x00, 0x00, 0xee, pixmap);
	colors->wall         = GP_RGBToPixmapPixel(0x66, 0x66, 0x66, pixmap);
	colors->moveable     = GP_RGBToPixmapPixel(0xff, 0xff, 0x60, pixmap);
	colors->edible       = GP_RGBToPixmapPixel(0xff, 0x7f, 0x50, pixmap);
	colors->particle     = GP_RGBToPixmapPixel(0xff, 0xff, 0x00, pixmap);
	colors->particle_dir = GP_RGBToPixmapPixel(0xff, 0x44, 0x00, pixmap);
}

static void render_none(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);
}

static void render_player(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);

	GP_FillCircle(render->pixmap, x + w/2, y + w/2, w/2 - 1, colors.player);
	GP_FillRing(render->pixmap, x + w/2, y + w/2, w/2 - 1, w/2 - 2, colors.frames);
}

static void render_wall(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.wall);

	if (!(elem->flags & BOGOMAN_LEFT)) {
		GP_VLineXYH(render->pixmap, x, y, w, colors.frames);
		GP_VLineXYH(render->pixmap, x+1, y, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_RIGHT)) {
		GP_VLineXYH(render->pixmap, x + w - 1, y, w, colors.frames);
		GP_VLineXYH(render->pixmap, x + w - 2, y, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_UP)) {
		GP_HLineXYW(render->pixmap, x, y, w, colors.frames);
		GP_HLineXYW(render->pixmap, x, y+1, w, colors.frames);
	}

	if (!(elem->flags & BOGOMAN_DOWN)) {
		GP_HLineXYW(render->pixmap, x, y + w - 1, w, colors.frames);
		GP_HLineXYW(render->pixmap, x, y + w - 2, w, colors.frames);
	}
}

static void render_diamond(struct bogoman_render *render,
                           unsigned int x, unsigned int y,
                           struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);

	(void) elem;

	GP_FillTetragon(render->pixmap, x + w/2, y, x + w - 1, y + w/2,
	                x + w/2, y + w - 1, x, y + w/2, colors.diamond);

	GP_Tetragon(render->pixmap, x + w/2, y, x + w - 1, y + w/2,
	            x + w/2, y + w - 1, x, y + w/2, colors.frames);
	GP_Tetragon(render->pixmap, x + w/2, y+1, x + w - 2, y + w/2,
	            x + w/2, y + w - 2, x+1, y + w/2, colors.frames);
}

static void render_moveable(struct bogoman_render *render,
                            unsigned int x, unsigned int y,
                            struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);

	GP_FillRectXYWH(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.moveable);
	GP_RectXYWH(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.frames);
	GP_RectXYWH(render->pixmap, x + 2, y + 2, w - 4, w - 4, colors.frames);
}

static void render_edible(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);

	GP_FillRectXYWH(render->pixmap, x + 1, y + 1, w - 2, w - 2, colors.edible);
}

static void render_particle(struct bogoman_render *render,
                            unsigned int x, unsigned int y,
                            struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;
	int dir = elem->flags & BOGOMAN_DIRECTION_MASK;

	GP_FillRectXYWH(render->pixmap, x, y, w, w, colors.bg);

	switch (elem->flags & ~BOGOMAN_DIRECTION_MASK) {
	case BOGOMAN_PARTICLE_ROUND:
		GP_FillCircle(render->pixmap, x + w/2, y + w/2, w/2-1, colors.particle);
		GP_FillRing(render->pixmap, x + w/2, y + w/2, w/2 - 1, w/2 - 2, colors.frames);
	break;
	case BOGOMAN_PARTICLE_SQUARE:
		GP_FillRectXYWH(render->pixmap, x+1, y+1, w-2, w-2, colors.particle);
		GP_RectXYWH(render->pixmap, x+1, y+1, w-2, w-2, colors.frames);
		GP_RectXYWH(render->pixmap, x+2, y+2, w-4, w-4, colors.frames);
	break;
	}

	switch (dir) {
	case BOGOMAN_LEFT:
		GP_FillTriangle(render->pixmap, x + w/4, y + w/2,
		                x + 5*w/8, y + w/4, x + 5*w/8, y + 3*w/4, colors.particle_dir);
		GP_Triangle(render->pixmap, x + w/4, y + w/2,
		            x + 5*w/8, y + w/4, x + 5*w/8, y + 3*w/4, colors.frames);
	break;
	case BOGOMAN_RIGHT:
		GP_FillTriangle(render->pixmap, x + 3*w/4, y + w/2,
		                x + 3*w/8, y + w/4, x + 3*w/8, y + 3*w/4, colors.particle_dir);
		GP_Triangle(render->pixmap, x + 3*w/4, y + w/2,
		            x + 3*w/8, y + w/4, x + 3*w/8, y + 3*w/4, colors.frames);
	break;
	case BOGOMAN_UP:
		GP_FillTriangle(render->pixmap, x + w/2, y + w/4,
		                x + w/4, y + 5*w/8, x + 3*w/4, y + 5*w/8, colors.particle_dir);
		GP_Triangle(render->pixmap, x + w/2, y + w/4,
		            x + w/4, y + 5*w/8, x + 3*w/4, y + 5*w/8, colors.frames);
	break;
	case BOGOMAN_DOWN:
		GP_FillTriangle(render->pixmap, x + w/2, y + 3*w/4,
		                x + w/4, y + 3*w/8, x + 3*w/4, y + 3*w/8, colors.particle_dir);
		GP_Triangle(render->pixmap, x + w/2, y + 3*w/4,
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
		GP_BackendUpdateRect(render->backend, cx, cy,
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
		GP_Fill(render->pixmap, colors.bg);

	for (y = render->map_x_offset; y < render->map->h; y++) {
		for (x = render->map_x_offset; x < render->map->w; x++)
			render_elem(render, x, y, flags);
	}

	if (flags & BOGOMAN_RENDER_ALL && render->backend)
		GP_BackendFlip(render->backend);
}
