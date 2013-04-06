%include "../common.i"
%module(package="gfxprim.text") c_text

%{
#include "text/GP_Text.h"
#include "text/GP_Font.h"
%}

%import ../core/core.i

%ignore GP_GlyphBitmap::bitmap;
%ignore GP_FontFace::glyph_offsets;
%include "GP_Text.h"
%include "GP_Font.h"
%include "GP_DefaultFont.h"
%include "GP_TextMetric.h"
%include "GP_TextStyle.h"

