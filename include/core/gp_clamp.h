// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Fast clamping.

  */

#ifndef CORE_GP_CLAMP_H
#define CORE_GP_CLAMP_H

#ifdef __GNUC__
# define GP_IS_CONSTANT(x) __builtin_constant_p(x)
#else
# define GP_IS_CONSTANT(x) 0
#endif

#define GP_CLAMP_GENERIC(val, min, max) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(max) gp_max__ = (max); \
	typeof(min) gp_min__ = (min); \
	gp_val__ = gp_val__ < gp_min__ ? gp_min__ : gp_val__; \
	gp_val__ = gp_val__ > gp_max__ ? gp_max__ : gp_val__; \
	gp_val__; \
})

/*
 * Special case clamping for 8 bit values, which is sometimes faster.
 *
 * We first look if the number needs to be clamped and then do accordingly.
 */
#define GP_CLAMP_INT_0_255(val) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(val) _mask = ~0xff; \
	(gp_val__ & _mask) ? (gp_val__ < 0 ? 0 : 255) : gp_val__; \
})

#define GP_CLAMP(val, min, max) ({ \
	(GP_IS_CONSTANT(min) && GP_IS_CONSTANT(max) && \
	 min == 0 && max == 255) ? \
	 GP_CLAMP_INT_0_255(val) : \
	 GP_CLAMP_GENERIC(val, min, max); \
})

#define GP_CLAMP_DOWN(val, max) ({ \
	typeof(val) gp_val__ = (val); \
	typeof(val) gp_max__ = (max); \
	gp_val__ > gp_max__ ? gp_max__ : gp_val__; \
})

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
