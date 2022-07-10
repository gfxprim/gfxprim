// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Keymap takes key press events and generates unicode letter events.

 */

#ifndef INPUT_GP_KEYMAP_H
#define INPUT_GP_KEYMAP_H

#include <input/gp_types.h>

struct gp_keymap {
	/*
	 * @self A keymap
	 * @queue An event queue to push unicode events to
	 * @ev A key event
	 * @return non-zero if event was dead-key zero otherwise.
	 */
	int (*event_key)(gp_keymap *self, gp_event_queue *queue, gp_event *ev);
	void *priv;
	void *priv2;
};

static inline int gp_keymap_event_key(gp_keymap *self, gp_event_queue *queue, gp_event *ev)
{
	return self->event_key(self, queue, ev);
}

gp_keymap *gp_keymap_load(const char *name);

gp_keymap *gp_keymap_json_load(const char *json_str);

void gp_keymap_free(gp_keymap *self);

#endif /* INPUT_GP_KEYMAP_H */
