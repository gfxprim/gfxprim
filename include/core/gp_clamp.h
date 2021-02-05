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

#define GP_CLAMP_GENERIC(val, min, max) ({  \
	typeof(val) _val = (val);           \
	typeof(max) _max = (max);           \
	typeof(min) _min = (min);           \
	_val = _val < _min ? _min : _val;   \
	_val = _val > _max ? _max : _val;   \
	_val;                               \
})

/*
 * Special case clamping for 8 bit values, which is sometimes faster.
 *
 * We first look if the number needs to be clamped and then do accordingly.
 */
#define GP_CLAMP_INT_0_255(val) ({                    \
	typeof(val) _val = (val);                     \
	typeof(val) _mask = ~0xff;                    \
	(_val & _mask) ? (_val < 0 ? 0 : 255) : _val; \
})

#define GP_CLAMP(val, min, max) ({                     \
	(GP_IS_CONSTANT(min) && GP_IS_CONSTANT(max) && \
	 min == 0 && max == 255) ?                     \
	 GP_CLAMP_INT_0_255(val) :                     \
	 GP_CLAMP_GENERIC(val, min, max);              \
})

#define GP_CLAMP_DOWN(val, max) ({        \
	typeof(val) _val = (val);         \
	typeof(val) _max = (max);         \
	_val = _val > _max ? _max : _val; \
	_val;                             \
})

#endif /* CORE_GP_CLAMP_H */
