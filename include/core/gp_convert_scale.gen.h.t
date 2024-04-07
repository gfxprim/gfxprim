@ include header.t
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2013-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_convert_scale.gen.h
 * @brief Fast value scaling macros.
 */
@ def multcoef(s1, s2):
(0+{{'+'.join([hex(2 ** (i * s1)) for i in range((s2 + s1 - 1)//s1)])}})
@ end

/**
 * @brief Helper macros to transfer s1-bit value to s2-bit value.
 *
 * Efficient and accurate for both up- and downscaling.
 *
 * @warning GP_SCALE_VAL requires constants numbers as first two parameters.
 *
 * @param s1 Number of bits to scale from.
 * @param s2 Number of bits to scale to.
 * @param val A value to be scaled.
 *
 * @return A scaled value.
 */
#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )

@ for s1 in range(1,33):
@     for s2 in range(1,17):
@         if s2 > s1:
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) (((val) * {@ multcoef(s1, s2) @}) >> {{ (-s2) % s1 }})
@         else:
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})
