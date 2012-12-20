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

   Image list.

  */

#ifndef __IMAGE_LIST_H__
#define __IMAGE_LIST_H__

struct image_list;

/*
 * Takes NULL-terminated array of paths as parameter
 */
struct image_list *image_list_create(const char *args[]);

/*
 * Returns path to the current image.
 */
const char *image_list_img_path(struct image_list *self);

/*
 * Moves the current image direction images and returns pointer to current
 * path.
 */
const char *image_list_move(struct image_list *self, int direction);

#endif /* __IMAGE_LIST_H__ */
