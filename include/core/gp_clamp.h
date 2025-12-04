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
	typeof(val) gp_val_tmp = (val); \
	typeof(max) gp_max_tmp = (max); \
	typeof(min) gp_min_tmp = (min); \
	gp_val_tmp = gp_val_tmp < gp_min_tmp ? gp_min_tmp : gp_val_tmp; \
	gp_val_tmp = gp_val_tmp > gp_max_tmp ? gp_max_tmp : gp_val_tmp; \
	gp_val_tmp; \
})

#define GP_CLAMP_INT_0_255(val) ({ \
	typeof(val) gp_val_tmp = (val); \
	typeof(val) gp_mask_tmp = ~0xff; \
	(gp_val_tmp & gp_mask_tmp) ? (gp_val_tmp < 0 ? 0 : 255) : gp_val_tmp; \
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
	typeof(val) gp_val_tmp = (val); \
	typeof(val) gp_max_tmp = (max); \
	gp_val_tmp > gp_max_tmp ? gp_max_tmp : gp_val_tmp; \
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
	typeof(val) gp_val_tmp = (val); \
	typeof(add) gp_add_tmp = (add); \
	typeof(val) gp_max_tmp = (max); \
	typeof(val) gp_ret_tmp; \
	typeof(val) gp_add_neg_tmp = -gp_add_tmp; \
	if (gp_add_tmp > 0) \
		gp_ret_tmp =  gp_val_tmp + gp_add_tmp > gp_max_tmp ? gp_max_tmp : gp_val_tmp + gp_add_tmp; \
	else \
		gp_ret_tmp =  gp_val_tmp < gp_add_neg_tmp ? 0 : gp_val_tmp + gp_add_tmp; \
	gp_ret_tmp; \
})

#endif /* CORE_GP_CLAMP_H */
