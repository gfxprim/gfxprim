// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_ev_feedback.h
 * @brief An input event feedback, e.g. keyboard leds.
 *
 * Implements input device feedback, currently keyboard leds but can be
 * modified to include force feedback in joysticks as well.
 */

#ifndef INPUT_GP_EV_FEEDBACK_H
#define INPUT_GP_EV_FEEDBACK_H

#include <stdint.h>
#include <input/gp_types.h>

/**
 * @brief A bitfield of keyboard leds to manipulate.
 *
 * This is passed in the val field of the gp_ev_feedback_op structure.
 */
enum gp_ev_feedback_val_leds {
	/** @brief A num lock led. */
	GP_KBD_LED_NUM_LOCK = 0x01,
	/** @brief A caps lock led. */
	GP_KBD_LED_CAPS_LOCK = 0x02,
	/** @brief A scroll lock led. */
	GP_KBD_LED_SCROLL_LOCK = 0x04,
	/** @brief A compose led. */
	GP_KBD_LED_COMPOSE = 0x08,
	/** @brief A kana led. */
	GP_KBD_LED_KANA = 0x10,
};

/**
 * @brief A feedback operation to be done.
 *
 * This is passed in the op field of the gp_ev_feedback_op structure.
 */
enum gp_ev_feedback_op_type {
	/** @brief Turns on leds in the val mask. */
	GP_EV_LEDS_ON,
	/** @brief Turns off leds in the val mask. */
	GP_EV_LEDS_OFF,
	/** @brief Returns leds status in the val field. */
	GP_EV_LEDS_GET,
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
 * @param root A pointer to a root of the feedbacks list.
 * @param op An operation.
 */
void gp_ev_feedback_set_all(gp_ev_feedback *root, gp_ev_feedback_op *op);

/**
 * @brief Adds a feedback to a feedback queue.
 *
 * @param root A pointer to a root of the feedbacks list.
 * @param self A feedback to be registered.
 */
void gp_ev_feedback_register(gp_ev_feedback **root, gp_ev_feedback *self);

/**
 * @brief Removes a feedback from a feedback queue.
 *
 * @param root A pointer to a root of the feedbacks list.
 * @param self A feedback to be unregistered.
 */
void gp_ev_feedback_unregister(gp_ev_feedback **root, gp_ev_feedback *self);

#endif /* INPUT_GP_EV_FEEDBACK_H */
