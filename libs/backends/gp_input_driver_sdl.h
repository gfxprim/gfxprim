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

/*

  SDL input driver. Translates SDL events to GFXprim events.

 */

#ifndef INPUT_GP_INPUT_DRIVER_SDL_H
#define INPUT_GP_INPUT_DRIVER_SDL_H

#include <stdint.h>
#include <SDL/SDL.h>

/*
 * Converts SDL event to GFXprim evevt and puts it into the queue.
 */
void gp_input_driver_sdl_event_put(gp_event_queue *event_queue, SDL_Event *ev);

#endif /* INPUT_GP_INPUT_DRIVER_SDL_H */
