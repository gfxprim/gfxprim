// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Legacy KBD input driver.

 */

#ifndef INPUT_GP_INPUT_DRIVER_KBD_H
#define INPUT_GP_INPUT_DRIVER_KBD_H

#include <input/gp_types.h>

/*
 * Converts KBD event to GFXprim event and puts it into the queue.
 */
void gp_input_driver_kbd_event_put(gp_event_queue *event_queue,
                                   unsigned char ev);

#endif /* INPUT_GP_INPUT_DRIVER_KBD_H */
