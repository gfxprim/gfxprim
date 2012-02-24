%module(package="gfxprim.text") text_c

%{
#include "text/GP_Text.h"
#include "text/GP_Font.h"
%}

#define __attribute__(X)

%import ../core/core.i

%include <stdint.i>

%nodefaultctor;

%ignore GP_GlyphBitmap::bitmap;
%ignore GP_FontFace::glyph_offsets;
%include "GP_Text.h"
%include "GP_Font.h"
%include "GP_DefaultFont.h"
%include "GP_TextMetric.h"
%include "GP_TextStyle.h"

