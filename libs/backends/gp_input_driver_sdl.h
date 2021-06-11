// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  SDL input driver. Translates SDL events to GFXprim events.

 */

#ifndef INPUT_GP_INPUT_DRIVER_SDL_H
#define INPUT_GP_INPUT_DRIVER_SDL_H

#include <stdint.h>

/*
 * Converts SDL event to GFXprim evevt and puts it into the queue.
 */
void gp_input_driver_sdl_event_put(gp_event_queue *event_queue, SDL_Event *ev);

#endif /* INPUT_GP_INPUT_DRIVER_SDL_H */
