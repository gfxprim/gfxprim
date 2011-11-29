%% extends 'base.c.t'

/*
 * Maybe opts, adds comma on the left side
 */
{% macro maybe_opts_l(opts) %}{% if opts %}, {{ opts }}{% endif %}{% endmacro %}

/*
 * Maybe opts, adds comma on the right side.
 */
{% macro maybe_opts_r(opts) %}{% if opts %}{{ opts }}, {% endif %}{% endmacro %}
