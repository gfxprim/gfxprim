// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Implements input device feedback, mostly keyboard leds but can include force
  feedback in joysticks as well.

 */

#ifndef INPUT_GP_EV_FEEDBACK_H
#define INPUT_GP_EV_FEEDBACK_H

#include <stdint.h>
#include <input/gp_types.h>

/**
 * A bitfield of keyboard leds to manipulate.
 *
 * This is passed in the val field of the gp_ev_feedback_op structure.
 */
enum gp_ev_feedback_val_leds {
	GP_KBD_LED_NUM_LOCK = 0x01,
	GP_KBD_LED_CAPS_LOCK = 0x02,
	GP_KBD_LED_SCROLL_LOCK = 0x04,
	GP_KBD_LED_COMPOSE = 0x08,
	GP_KBD_LED_KANA = 0x10,
};

/**
 * An feedback operation to be done.
 *
 * This is passed in the op field of the gp_ev_feedback_op structure.
 */
enum gp_ev_feedback_op_type {
	GP_EV_LEDS_ON,  /* Turns on leds in the val mask */
	GP_EV_LEDS_OFF, /* Turns off leds in the val mask */
	GP_EV_LEDS_GET, /* Returns leds status in the val field */
};

struct gp_ev_feedback_op {
	uint8_t op;
	uint8_t val;
};

struct gp_ev_feedback {
	int (*set_get)(gp_ev_feedback *self, gp_ev_feedback_op *op);
	gp_ev_feedback *next;
};

/**
 * @brief Passes an opeartion to all registered feedbacks.
 *
 * @root A pointer to a root of the feedbacks list.
 * @op An operation.
 */
void gp_ev_feedback_set_all(gp_ev_feedback *root, gp_ev_feedback_op *op);

/**
 * @brief Adds a feedback to a feedback queue.
 *
 * @root A pointer to a root of the feedbacks list.
 * @self A feedback to be registered.
 */
void gp_ev_feedback_register(gp_ev_feedback **root, gp_ev_feedback *self);

/**
 * @brief Removes a feedback from a feedback queue.
 *
 * @root A pointer to a root of the feedbacks list.
 * @self A feedback to be unregistered.
 */
void gp_ev_feedback_unregister(gp_ev_feedback **root, gp_ev_feedback *self);

#endif /* INPUT_GP_EV_FEEDBACK_H */
