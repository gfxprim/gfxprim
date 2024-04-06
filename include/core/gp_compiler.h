// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_compiler.h
 * @brief A compiler dependent macros.
 */

#ifndef CORE_GP_COMPILER_H
#define CORE_GP_COMPILER_H

#if __GNUC__
# define GP_IS_CONSTANT(x) __builtin_constant_p(x)
# define GP_WUR __attribute__((warn_unused_result))
# define GP_FMT_PRINTF(fmt, list) __attribute__((format(printf, fmt, list)))
# define GP_FMT_SCANF(fmt, list) __attribute__((format(scanf, fmt, list)))
#else
/**
 * @brief Expands to warn_unused_result attribute when supported by the compiler.
 */
# define GP_WUR
/**
 * @brief Expands to format printf attribute when supported by the compiler.
 */
# define GP_FMT_PRINTF(fmt, list)
/**
 * @brief Expands to format scanf attribute when supported by the compiler.
 */
# define GP_FMT_SCANF(fmt, list)
/**
 * @brief Expands to __buildin_constant_p() if supported by compiler.
 */
# define GP_IS_CONSTANT(x) 0
#endif

#endif /* CORE_GP_COMPILER_H */
