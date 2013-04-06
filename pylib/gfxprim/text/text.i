%include "../common.i"
%module(package="gfxprim.text") c_text

%{
#include "text/GP_Text.h"
#include "text/GP_Font.h"
#include "core/GP_Debug.h"
%}

%import ../core/core.i

%ignore GP_GlyphBitmap::bitmap;
%ignore GP_FontFace::glyph_offsets;
%include "GP_Text.h"
%include "GP_Font.h"
%include "GP_DefaultFont.h"
%include "GP_TextMetric.h"

/* GP_TextStyle wrappers */
%extend GP_TextStyle {
  ~GP_TextStyle() {
    GP_DEBUG(2, "[wrapper] GP_TextStyleFree (%p)", $self);
    free($self);
  }
  GP_TextStyle(const struct GP_FontFace *font, int xspace, int yspace,
               int xmul, int ymul, int spacing) {
    GP_TextStyle *style = malloc(sizeof(GP_TextStyle));

    GP_DEBUG(2, "[wrapper] GP_TextStyleAlloc (%p)", style);

    if (style == NULL)
      return NULL;

    style->font = font;
    style->pixel_xspace = xspace;
    style->pixel_yspace = yspace;
    style->pixel_xmul = xmul;
    style->pixel_ymul = ymul;
    style->char_xspace = spacing;

    return style;
  }
}

%include "GP_TextStyle.h"
