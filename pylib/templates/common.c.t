%% extends 'base.c.t'

/*
 * Maybe opts, adds comma on the left side
 */
{% macro maybe_opts_l(opts) %}{% if opts %}, {{ opts }}{% endif %}{% endmacro %}

/*
 * Maybe opts, adds comma on the right side.
 */
{% macro maybe_opts_r(opts) %}{% if opts %}{{ opts }}, {% endif %}{% endmacro %}

/*
 * Converts channels to params
 */
{% macro expand_chanslist(pt, pref="", suff="") %}{{ pref+pt.chanslist[0][0]+suff }}{% for c in pt.chanslist[1:] %}, {{ pref+c[0]+suff }}{% endfor %}{% endmacro %}
/*
 * Clamps n-bits value
 */
%%macro clamp_val(val, bits)
if ({{ val }} > {{ 2 ** bits - 1 }}) {{ val }} = {{ 2 ** bits - 1}};
%% endmacro

{% macro gamma_in_bits(size) %}{% if size + 2 > 8 %}16{% else %}8{% endif %}{% endmacro %}
{% macro gamma_out_bits(size) %}{% if size > 8 %}16{% else %}8{% endif %}{% endmacro %}

/*
 * Prepares pointers to Gamma tables.
 */
%% macro fetch_gamma_tables(pt, ctx, pref="", suff="")
	/* prepare Gamma tables */
	%% for c in pt.chanslist
	uint{{ gamma_in_bits(c[2]) }}_t *{{ pref+c[0] }}_2_LIN{{ suff }} = NULL;
	%% endfor

	%% for c in pt.chanslist
	uint{{ gamma_out_bits(c[2]) }}_t *{{ pref+c[0] }}_2_GAMMA{{ suff }} = NULL;
	%% endfor

	%% set i = 0
	if ({{ ctx }}->gamma) {
		%% for c in pt.chanslist
		{{ pref+c[0] }}_2_LIN{{ suff }} = {{ ctx }}->gamma->tables[{{ i }}]->u{{ gamma_in_bits(c[2]) }};
		%% set i = i + 1
		%% endfor

		%% set i = len(pt.chanslist)
		%% for c in pt.chanslist
		{{ pref+c[0] }}_2_GAMMA{{ suff }} = {{ ctx }}->gamma->tables[{{ i }}]->u{{ gamma_out_bits(c[2]) }};
		%% set i = i + 1
		%% endfor
	}
%% endmacro

