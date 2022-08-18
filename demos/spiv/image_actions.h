// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Image Actions.

   Image action is an arbitrary command with printf-like syntax for image name,
   format and path.

   %f - image path
   %F - shell escaped path

   %n - image name
   %N - escaped image name

  */

#ifndef IMAGE_ACTIONS_H__
#define IMAGE_ACTIONS_H__

void image_action_set(unsigned int action, const char *cmd);

int image_action_run(unsigned int action, const char *img_path);

#endif /* IMAGE_ACTIONS_H__ */
