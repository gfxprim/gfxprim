// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_linux_fb.h
 * @brief A Linux framebuffer backend.
 */
#ifndef BACKENDS_GP_LINUX_FB_H
#define BACKENDS_GP_LINUX_FB_H

#include <backends/gp_backend.h>

/** @brief Linux framebuffer init flags. */
enum gp_linux_fb_flags {
	/**
	 * @brief Use KBD to get input events.
	 *
	 * Does not support anything but very basic keyboard use
	 * GP_FB_INPUT_LINUX instead.
	 */
	GP_FB_INPUT_KBD = 0x01,
	/**
	 * @brief Use shadow framebuffer for drawing.
	 *
	 * Should be enabled unless you are on a very memory constrained hardware.
	 */
	GP_FB_SHADOW = 0x02,
	/**
	 * @brief Allocate new console, if not set current is used.
	 */
	GP_FB_ALLOC_CON = 0x04,
	/**
	 * @brief Use Linux input for keyboards and mouse.
	 *
	 * This is the modern alternative to KBD.
	 */
	GP_FB_INPUT_LINUX = 0x08,
};

/**
 * @brief Linux framebuffer backend initialization.
 *
 * @param path A path to a framebuffer device, e.g. "/dev/fb0" for first framebuffer.
 * @param flags A bitwise combination of framebuffer init flags.
 * @return A newly allocated and initialized backend or NULL in a case of a failure.
 */
gp_backend *gp_linux_fb_init(const char *path, enum gp_linux_fb_flags flags);

#endif /* BACKENDS_GP_LINUX_FB_H */
