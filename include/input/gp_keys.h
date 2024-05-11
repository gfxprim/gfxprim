// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_keys.h
 * @brief A key utility functions.
 */

#ifndef INPUT_GP_KEYS_H
#define INPUT_GP_KEYS_H

#include <input/gp_event_keys.h>

/**
 * @brief Parses a key name and returns key value.
 *
 * The key names are not case sensitive, e.g. "KeyWWW" and "keyWWW" both
 * return the same key code.
 *
 * @param name A key name such as "KeyEnter" or "KeyA"
 * @return A key code or -1 when match was not found.
 */
int gp_ev_key_val(const char *name);

/**
 * @brief Returns a key name.
 *
 * @param val A key value.
 * @return A key name.
 */
const char *gp_ev_key_name(unsigned int val);

#endif /* INPUT_GP_KEYS_H */
