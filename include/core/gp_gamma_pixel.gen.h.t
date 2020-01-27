@ include header.t
/*
 * Gamma correction for pixels
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/GP_Types.h>
#include <core/gp_gamma_correction.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
@         idx = 0

@         for c in pt.chanslist:

/*
 * Converts gamma encoded pixel value to linear value.
 *
 * Parameters are, converted value and gp_gamma structure.
 */
#define gp_gamma2lin_{{ pt.name }}_{{ c[0] }}(val, gamma) ({ \
@             if c[2]  > 6:
	gamma->tables[{{ idx }}]->u16[val]; \
@             else:
	gamma->tables[{{ idx }}]->u8[val]; \
@             end
})

/*
 * Converts linear encoded pixel into gamma encoded pixel.
 *
 * Parameters are, converted value and gp_gamma structure.
 */
#define gp_lin2gamma_{{ pt.name }}_{{ c[0] }}(val, gamma) ({ \
@             if c[2] > 8:
	gamma->tables[{{ len(pt.chanslist) + idx}}]->u16[val]; \
@             else:
	gamma->tables[{{ len(pt.chanslist) + idx}}]->u8[val]; \
@             end
})

static inline gp_gamma_table *gp_gamma_table_{{ pt.name }}_{{ c[0] }}(gp_gamma *gamma)
{
	return gamma->tables[{{ idx }}];
}

static inline gp_gamma_table *gp_gamma_inverse_table_{{ pt.name }}_{{ c[0] }}(gp_gamma *gamma)
{
	return gamma->tables[{{ len(pt.chanslist) + idx }}];
}

@             idx = idx + 1
@ end

#define gp_gamma2lin(val, chan_bits, gamma_table) ({ \
#if chan_bits > 6
	gamma_table->u16[val] \
#else
	gamma_table->u8[val] \
#endif
})

#define gp_lin2gamma(val, chan_bits, gamma_table) ({ \
#if chan_bits > 8
	gamma_table->table16[val] \
#else
	gamma_table->table8[val] \
#endif
})
