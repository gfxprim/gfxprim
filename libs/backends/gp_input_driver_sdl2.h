// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  SDL2 input driver. Translates SDL2 events to GFXprim events.

 */

#ifndef INPUT_GP_INPUT_DRIVER_SDL2_H
#define INPUT_GP_INPUT_DRIVER_SDL2_H

#include <stdint.h>
#include <SDL2/SDL.h>

/*
 * Converts SDL2 event to GFXprim event and puts it into the queue.
 */
void gp_input_driver_sdl2_event_put(gp_event_queue *event_queue, SDL_Event *ev);

#endif /* INPUT_GP_INPUT_DRIVER_SDL2_H */
