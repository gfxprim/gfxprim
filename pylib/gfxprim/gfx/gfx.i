%include "../common.i"
%module(package="gfxprim.gfx") gfx_c

%{
#include "gfx/GP_Gfx.h"
%}

%import ../core/core.i

%include "GP_Gfx.h"

/* Listed in GP_Gfx.h: */
%include "GP_Fill.h"
%include "GP_HLine.h"
%include "GP_VLine.h"
%include "GP_Line.h"
%include "GP_Rect.h"
%include "GP_Triangle.h"
%include "GP_Tetragon.h"
%include "GP_Circle.h"
/* %include "GP_CircleSeg.h"  -- missing symbols */
%include "GP_Ellipse.h"
%include "GP_Arc.h"
%include "GP_Polygon.h"
%include "GP_Symbol.h"

%include "GP_PutPixelAA.h"
%include "GP_VLineAA.h"
%include "GP_HLineAA.h"
%include "GP_LineAA.h"
%include "GP_RectAA.h"

