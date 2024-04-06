// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_debug.h
 * @brief A debug message layer.
 *
 * Many places of the library uses debug messages to report warnings, bugs, or
 * generally important events i.e. pixmap has been allocated, filter function
 * has been called, etc.
 *
 * Debug messages are printed into the stderr and could be redirected to custom
 * handler.
 *
 * The debug level can be set by exporting an enviroment variable GP_DEBUG=level.
 *
 * The verbosity of the messages could be changed by the debug level. The debug
 * level is an unsigned integer (by default set to '0') and only messages that have
 * debug level lower or equal to debug level are printed.
 *
 * There are few special debug message types with negative debug level (that
 * means that they are always printed), and as so these are used on various error
 * conditions, see below for more information.
 */

#ifndef CORE_GP_DEBUG_H
#define CORE_GP_DEBUG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <core/gp_compiler.h>

/**
 * @brief A debug level constants.
 *
 * Messages with debug level 0 are always printed (you should generally avoid
 * using them unless you wan't user to see the message.)
 *
 * Debug level 1 should be used on object initalization and generally rare and
 * important events.
 *
 * Debug level > 1 is intended for more verbose reporting, like inner cycles
 * or loop debugging.
 *
 * Debug levels with negative level are special.
 */
enum gp_debug_type {
	/**
	 * @brief Not implemented yet
	 *
	 * Used in stub functions.
	 */
	GP_DEBUG_TODO  = -1,
	/**
	 * @brief A warning
	 *
	 * Generally a problem that can be ignored.
	 */
	GP_DEBUG_WARN  = -2,
	/**
	 * @brief A library bug
	 *
	 * Library got into an inconsistent state but stil attempts to
	 * continue.
	 */
	GP_DEBUG_BUG   = -3,
	/**
	 * @brief A fatal condition.
	 *
	 * - Initialization failed
	 * - Library foo not compiled in
	 */
	GP_DEBUG_FATAL = -4,
};

#define GP_DEFAULT_DEBUG_LEVEL 0

/**
 * @brief A debug printf-like macro.
 *
 * @param level A debug level usually between 1 and 10
 * @param ... A printf like format string and parameters.
 */
#define GP_DEBUG(level, ...) \
	gp_debug_print(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief A debug TODO printf-like macro.
 *
 * @param ... A printf like format string and parameters.
 */
#define GP_TODO(...) \
	gp_debug_print(GP_DEBUG_TODO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief A debug WARN printf-like macro.
 *
 * @param ... A printf like format string and parameters.
 */
#define GP_WARN(...) \
	gp_debug_print(GP_DEBUG_WARN, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief A debug BUG printf-like macro.
 *
 * @param ... A printf like format string and parameters.
 */
#define GP_BUG(...) \
	gp_debug_print(GP_DEBUG_BUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief A debug FATAL printf-like macro.
 *
 * @param ... A printf like format string and parameters.
 */
#define GP_FATAL(...) \
	gp_debug_print(GP_DEBUG_FATAL, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * @brief A debug printf handler.
 *
 * @attention Do not use directly use the GP_DEBUG(), GP_WARN(), ... macros instead.
 *
 * @param level A debug level for the mesasge.
 * @param file A file name for the source file the message came from.
 * @param function A function the message came from.
 * @param line A line in the source file the message came from.
 * @param fmt A printf-like format string.
 * @param ... A printf-like paramaters.
 */
void gp_debug_print(int level, const char *file, const char *function, int line,
                   const char *fmt, ...) GP_FMT_PRINTF(5, 6);

/**
 * @brief Sets current debug level.
 *
 * The default debug level is 0 that means that all messages with level up to 0
 * are printed. To get more verbose debugging pass a value between 1 and 10.
 *
 * @param level A new debug level.
 */
void gp_set_debug_level(unsigned int level);

/**
 * @brief Returns current debug level.
 *
 * @return A current debug level.
 */
unsigned int gp_get_debug_level(void);

/**
 * @brief Custom debug message handler structure.
 */
struct gp_debug_msg {
	/** @brief Message debug level, see #gp_debug_type */
	int level;
	/** @brief A source file the message came from. */
	const char *file;
	/** @brief A function the message came from. */
	const char *fn;
	/** @brief A line the source file the message came from */
	unsigned int line;
	/** @brief The debug message. */
	const char *msg;
};

/**
 * @brief Redirects debug message into a custom handler.
 *
 * @param handler A new debug handler. Pass NULL to disable redirection.
 */
void gp_set_debug_handler(void (*handler)(const struct gp_debug_msg *msg));

#endif /* CORE_GP_DEBUG_H */
