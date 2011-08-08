%% extends "base.h.t"

{% block descr %}Specialized blit functions and macros.{% endblock %}

%% block body

%% for ps in pixelsizes
/*** Blit preserving type for {{ ps.suffix }} ***
 * Assumes the contexts to be of the right types and sizes
 * Ignores transformations and clipping */
void GP_Blit_{{ ps.suffix }}(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,
			GP_Context *c2, GP_Coord x2, GP_Coord y2);

%% endfor

%% endblock body
