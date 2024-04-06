// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_clamp.h
  * @brief Fast value clamping.
  */
#ifndef CORE_GP_CLAMP_H
#define CORE_GP_CLAMP_H

#include <core/gp_compiler.h>

#define GP_CLAMP_GENERIC(val, min, max) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(max) gp_max__ = (max); \
	typeof(min) gp_min__ = (min); \
	gp_val__ = gp_val__ < gp_min__ ? gp_min__ : gp_val__; \
	gp_val__ = gp_val__ > gp_max__ ? gp_max__ : gp_val__; \
	gp_val__; \
})

#define GP_CLAMP_INT_0_255(val) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(val) _mask = ~0xff; \
	(gp_val__ & _mask) ? (gp_val__ < 0 ? 0 : 255) : gp_val__; \
})

/**
 * @brief Clamps a value.
 *
 * The resulting value will fit into [min, max] interval.
 *
 * @param val A value to be clamped.
 * @param min A lower limit.
 * @param max An upper limit.
 *
 * @return A clamped value.
 */
#define GP_CLAMP(val, min, max) ({ \
	(GP_IS_CONSTANT(min) && GP_IS_CONSTANT(max) && \
	 min == 0 && max == 255) ? \
	 GP_CLAMP_INT_0_255(val) : \
	 GP_CLAMP_GENERIC(val, min, max); \
})

/**
 * @brief Clamps down a value.
 *
 * @param val A value to be clamped.
 * @param max An uppper limit.
 *
 * @return A clamped down value.
 */
#define GP_CLAMP_DOWN(val, max) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(val) gp_max__ = (max); \
	gp_val__ > gp_max__ ? gp_max__ : gp_val__; \
})

/**
 * @brief Saturated addition.
 *
 * The resulting value will fit into the [0, max] interval.
 *
 * @param val A positive value to add to.
 * @param add A value to be added may be negative.
 * @param max An upper limit for the addition.
 *
 * @return A saturated addition.
 */
#define GP_SAT_ADD(val, add, max) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(add) gp_add__ = (add); \
	typeof(val) gp_max__ = (max); \
	typeof(val) gp_ret__; \
	typeof(val) gp_add_neg__ = -gp_add__; \
	if (gp_add__ > 0) \
		gp_ret__ =  gp_val__ + gp_add__ > gp_max__ ? gp_max__ : gp_val__ + gp_add__; \
	else \
		gp_ret__ =  gp_val__ < gp_add_neg__ ? 0 : gp_val__ + gp_add__; \
	gp_ret__; \
})

#endif /* CORE_GP_CLAMP_H */
