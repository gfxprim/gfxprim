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
{% macro expand_chanslist(chlist) %} {{ chlist[0][0] }}{% for i in chlist %}, {{ i[0] }}{% endfor %}{% endmacro %}

/*
 * Clamps n-bits value
 */
%%macro clamp_val(val, bits)
if ({{ val }} > {{ 2 ** bits - 1 }}) {{ val }} = {{ 2 ** bits - 1}};
%% endmacro
