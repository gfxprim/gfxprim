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
};

static struct render_colors colors;

static void init_colors(GP_Context *ctx, struct render_colors *colors)
{
	colors->bg       = GP_RGBToContextPixel(0xee, 0xee, 0xee, ctx);
	colors->player   = GP_RGBToContextPixel(0x00, 0xee, 0x00, ctx);
	colors->frames   = GP_RGBToContextPixel(0x00, 0x00, 0x00, ctx);
	colors->diamond  = GP_RGBToContextPixel(0x00, 0x00, 0xee, ctx);
	colors->wall     = GP_RGBToContextPixel(0x66, 0x66, 0x66, ctx);
	colors->moveable = GP_RGBToContextPixel(0xff, 0xff, 0x60, ctx);
	colors->edible   = GP_RGBToContextPixel(0xff, 0x7f, 0x50, ctx);
}

static void render_none(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	(void) elem;

	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.bg);
}

static void render_player(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;
	
	(void) elem;
	
	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.bg);
	GP_FillCircle(render->ctx, x + w/2, y + w/2, w/2 - 1, colors.player);
	GP_Circle(render->ctx, x + w/2, y + w/2, w/2 - 1, colors.frames);
}

static void render_wall(struct bogoman_render *render,
                        unsigned int x, unsigned int y,
			struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.wall);

	if (!(elem->flags & BOGOMAN_WALL_LEFT))
		GP_VLineXYH(render->ctx, x, y, w, colors.frames);
	
	if (!(elem->flags & BOGOMAN_WALL_RIGHT))
		GP_VLineXYH(render->ctx, x + w - 1, y, w, colors.frames);

	if (!(elem->flags & BOGOMAN_WALL_UP))
		GP_HLineXYW(render->ctx, x, y, w, colors.frames);

	if (!(elem->flags & BOGOMAN_WALL_DOWN))
		GP_HLineXYW(render->ctx, x, y + w - 1, w, colors.frames);
}

static void render_diamond(struct bogoman_render *render,
                           unsigned int x, unsigned int y,
                           struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.bg);
	
	GP_FillTetragon(render->ctx, x + w/2, y, x + w - 1, y + w/2,
	                x + w/2, y + w - 1, x, y + w/2, colors.diamond);

	GP_Tetragon(render->ctx, x + w/2, y, x + w - 1, y + w/2,
	            x + w/2, y + w - 1, x, y + w/2, colors.frames);
}

static void render_moveable(struct bogoman_render *render,
                            unsigned int x, unsigned int y,
                            struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;
	
	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.bg);

	GP_FillRectXYWH(render->ctx, x + 1, y + 1, w - 2, w - 2, colors.moveable);
	GP_RectXYWH(render->ctx, x + 1, y + 1, w - 2, w - 2, colors.frames);
}

static void render_edible(struct bogoman_render *render,
                          unsigned int x, unsigned int y,
                          struct bogoman_map_elem *elem)
{
	unsigned int w = render->map_elem_size;

	GP_FillRectXYWH(render->ctx, x, y, w, w, colors.bg);

	GP_FillRectXYWH(render->ctx, x + 1, y + 1, w - 2, w - 2, colors.edible);
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
}

void bogoman_render(struct bogoman_render *render, int flags)
{
	unsigned int x, y;

	//TODO: Hack
	init_colors(render->ctx, &colors);

	for (y = render->map_x_offset; y < render->map->h; y++) {
		for (x = render->map_x_offset; x < render->map->w; x++) {
			render_elem(render, x, y, flags);
		}
	}
}
