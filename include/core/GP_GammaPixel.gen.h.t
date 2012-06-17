%% extends "base.h.t"

{% block descr %}Gamma correction for pixels.{% endblock %}

%% block body

#include "GP_Context.h"
#include "GP_GammaCorrection.h"

%% for pt in pixeltypes
%% if not pt.is_unknown() and not pt.is_palette()
%% set idx = 0

%% for c in pt.chanslist

/*
 * Converts gamma encoded pixel value to linear value.
 *
 * Parameters are, converted value and GP_Gamma structure.
 */
#define GP_Gamma2Lin_{{ pt.name }}_{{ c[0] }}(val, gamma) ({ \
%% if c[2]  > 6
	gamma->tables[{{ idx }}]->u16[val]; \
%% else
	gamma->tables[{{ idx }}]->u8[val]; \
%% endif
})

/*
 * Converts linear encoded pixel into gamma encoded pixel.
 *
 * Parameters are, converted value and GP_Gamma structure.
 */
#define GP_Lin2Gamma_{{ pt.name }}_{{ c[0] }}(val, gamma) ({ \
%% if c[2] > 8
	gamma->tables[{{ len(pt.chanslist) + idx}}]->u16[val]; \
%% else
	gamma->tables[{{ len(pt.chanslist) + idx}}]->u8[val]; \
%% endif
})

static inline GP_GammaTable *GP_GammaTable_{{ pt.name }}_{{ c[0] }}(GP_Gamma *gamma)
{
	return gamma->tables[{{ idx }}];
}

static inline GP_GammaTable *GP_GammaInverseTable_{{ pt.name }}_{{ c[0] }}(GP_Gamma *gamma)
{
	return gamma->tables[{{ len(pt.chanslist) + idx }}];
}

%% set idx = idx + 1
%% endfor
%% endif
%% endfor

#define GP_Gamma2Lin(val, chan_bits, gamma_table) ({ \
#if chan_bits > 6
	gamma_table->u16[val] \
#else
	gamma_table->u8[val] \
#endif
})

#define GP_Lin2Gamma(val, chan_bits, gamma_table) ({ \
#if chan_bits > 8
	gamma_table->table16[val] \
#else
	gamma_table->table8[val] \
#endif
})

%% endblock body
