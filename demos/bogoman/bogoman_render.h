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

#ifndef __BOGOMAN_RENDER_H__
#define __BOGOMAN_RENDER_H__

struct bogoman_map;
struct GP_Pixmap;

struct bogoman_render {
	/* both in map elements */
	unsigned int map_x_offset;
	unsigned int map_y_offset;

	/* current map */
	struct bogoman_map *map;

	/* pixmap to be used for rendering */
	struct GP_Pixmap *pixmap;

	/* if not NULL is used to update screen */
	struct GP_Backend *backend;

	/* elem size in pixels */
	unsigned int map_elem_size;
};

enum bogonam_render_flags {
	/* renders all map elements, not only dirty ones */
	BOGOMAN_RENDER_ALL = 0x01,
	BOGOMAN_RENDER_DIRTY = 0x02,
};

void bogoman_render(struct bogoman_render *render, int flags);

#endif /* __BOGOMAN_RENDER_H__ */
