// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_common.h>
#include <stdint.h>

/**
 * Pre-computes linear resampling mapping.
 */

#ifndef GP_RESAMPLE_MAP
#define GP_RESAMPLE_MAP

#define MAPPING_FP_BITS 16
#define MAPPING_FP_HALF (1<<(MAPPING_FP_BITS-1))
#define MAPPING_FP_MASK ((1<<MAPPING_FP_BITS)-1)

static inline void mapping_precompute(uint32_t map[], uint16_t weights[],
                                      uint32_t src_size, uint32_t dst_size)
{
	uint64_t scale = (src_size << MAPPING_FP_BITS) / dst_size;
	uint64_t pos;

	for (pos = 0; pos < dst_size; pos++) {
		uint64_t m = (((pos<<MAPPING_FP_BITS) + MAPPING_FP_HALF) * scale)>>MAPPING_FP_BITS;

		if (m > MAPPING_FP_HALF)
			m -= MAPPING_FP_HALF;
		else
			m = 0;

		weights[pos] = (uint16_t)(m & MAPPING_FP_MASK);
		map[pos] = m >> MAPPING_FP_BITS;
	}
}

#endif /* GP_RESAMPLE_MAP */
