%% extends "base.h.t"

{% block descr %}Fast value scaling macros{% endblock %}

{% macro multcoef(s1, s2) -%}
(0{% for i in range((s2 + s1 - 1) // s1) %}+{{ 2 ** (i * s1) }}{% endfor %})
{%- endmacro %}

%% block body
/* 
 * Helper macros to transfer s1-bit value to s2-bit value.
 * Efficient and accurate for both up- and downscaling.
 * WARNING: GP_SCALE_VAL requires constants numbers as first two parameters
 */

#define GP_SCALE_VAL(s1, s2, val) ( GP_SCALE_VAL_##s1##_##s2(val) )

%% for s1 in range(1,9)
%% for s2 in range(1,9)
%% if s2>s1
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) (((val) * {{ multcoef(s1, s2) }}) >> {{ (-s2) % s1 }})
%% else
#define GP_SCALE_VAL_{{s1}}_{{s2}}(val) ((val) >> {{ s1 - s2 }})
%% endif
%% endfor
%% endfor

%% endblock body
