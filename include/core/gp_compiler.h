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
# define GP_LIKELY(cond) __builtin_expect(!!(cond), 1)
# define GP_UNLIKELY(cond) __builtin_expect(!!(cond), 0)
# define GP_UNUSED(x) (x)__attribute__ ((unused))
# define GP_PACKED    __attribute__ ((packed))
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
 * @brief Expands to __buildin_constant_p() if supported by the compiler.
 */
# define GP_IS_CONSTANT(x) 0
/**
 * @brief Expands to __buildin_expect() if supported by the compiler.
 */
# define GP_LIKELY(cond) (cond)
/**
 * @brief Expands to __buildin_expect() if supported by the compiler.
 */
# define GP_UNLIKELY(cond) (cond)
/**
 * @brief Expands to unused attribute if supported by the compiler.
 */
# define GP_UNUSED(x) x
/**
 * @brief Expand to packed attribute if supported by the compiler.
 */
# define GP_PACKED
#endif

#endif /* CORE_GP_COMPILER_H */
