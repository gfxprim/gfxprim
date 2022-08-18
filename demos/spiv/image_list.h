// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Image list.

  */

#ifndef IMAGE_LIST_H__
#define IMAGE_LIST_H__

struct image_list;

/*
 * Takes NULL-terminated array of paths as parameter
 */
struct image_list *image_list_create(const char *args[]);


void image_list_destroy(struct image_list *self);

/*
 * Returns path to the current image.
 */
const char *image_list_img_path(struct image_list *self);

/*
 * Moves the current image direction images and returns pointer to current
 * path.
 */
const char *image_list_move(struct image_list *self, int direction);

/*
 * If we are in directory:
 *  if direction > 0: move to its end and if allready there, to the next arg
 *  if direction < 0: move to its begining and if allready there, to the previous arg
 *
 * If we aren't in directory move by one in corresponding direction.
 */
const char *image_list_dir_move(struct image_list *self, int direction);

/*
 * Move to the first image in the list.
 */
const char *image_list_first(struct image_list *self);

/*
 * Move to the last image in the list.
 */
const char *image_list_last(struct image_list *self);

/*
 * Counts images int the list.
 */
unsigned int image_list_count(struct image_list *self);

/*
 * Returns current position in list, i.e. number between 0 and count - 1.
 */
unsigned int image_list_pos(struct image_list *self);

/*
 * Returns numbe of images in current dir or 0 if current arg is file.
 */
unsigned int image_list_dir_count(struct image_list *self);

/*
 * Returns current position in current dir or 0 if current arg is file.
 */
unsigned int image_list_dir_pos(struct image_list *self);

#endif /* IMAGE_LIST_H__ */
