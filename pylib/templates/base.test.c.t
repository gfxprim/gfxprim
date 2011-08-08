%% extends "base.c.t"


%% macro test(name, opts="")
GP_TEST({{ name }}, "{{ opts }}")
{
{{ caller() }}
}
GP_ENDTEST

%% endmacro


%% macro test_for_all_pixeltypes(name, opts="",
                                 palette=True, unknown=False, rgb=True,
				 alpha=True, gray=True)
%% for pt in pixeltypes
%% if unknown or not pt.is_unknown()
%% if palette or not pt.is_palette()
%% if rgb or not pt.is_rgb()
%% if alpha or not pt.is_alpha()
%% if gray or not pt.is_gray()
GP_TEST({{ name }}_{{ pt.name }}, "{{ opts }}")
{
{{ caller(pt) }}
}
GP_ENDTEST

%% endif
%% endif
%% endif
%% endif
%% endif
%% endfor
%% endmacro


%% macro test_for_all_pixelsizes(name, opts="")
%% for ps in pixelsizes
GP_TEST({{ name }}_{{ ps.suffix }}, "{{ opts }}")
{
{{ caller(ps) }}
}
GP_ENDTEST

%% endfor
%% endmacro
