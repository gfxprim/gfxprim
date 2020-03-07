// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __BOGOMAN_RENDER_H__
#define __BOGOMAN_RENDER_H__

struct bogoman_map;

struct bogoman_render {
	/* both in map elements */
	unsigned int map_x_offset;
	unsigned int map_y_offset;

	/* current map */
	struct bogoman_map *map;

	/* pixmap to be used for rendering */
	struct gp_pixmap *pixmap;

	/* if not NULL is used to update screen */
	struct gp_backend *backend;

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
