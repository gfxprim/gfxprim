%include "../common.i"
%module(package="gfxprim.text") c_text

%{
#include "text/GP_Text.h"
#include "text/GP_Font.h"
#include <core/gp_debug.h>
%}

%import ../core/core.i

%ignore gp_glyph::bitmap;
%ignore gp_font_face::glyph_offsets;
%ignore gp_print;
%ignore gp_vprint;
%include "GP_Text.h"
%include "GP_Font.h"
%include "GP_TextMetric.h"

%immutable gp_font_gfxprim;
%immutable gp_font_gfxprim_mono;
%immutable gp_font_tiny;
%immutable gp_font_tiny_mono;
%immutable gp_font_c64;
%immutable gp_font_haxor_narrow_15;
%immutable gp_font_haxor_narrow_bold_15;
%immutable gp_font_haxor_narrow_16;
%immutable gp_font_haxor_narrow_bold_16;
%immutable gp_font_haxor_narrow_17;
%immutable gp_font_haxor_narrow_bold_17;
%include "GP_Fonts.h"

/* GP_TextStyle wrappers */
%extend gp_text_style {
  ~gp_text_style() {
    GP_DEBUG(2, "[wrapper] gp_text_style_free (%p)", $self);
    free($self);
  }
  gp_text_style(const struct gp_font_face *font, int xspace, int yspace,
               int xmul, int ymul, int spacing) {
    gp_text_style *style = malloc(sizeof(gp_text_style));

    GP_DEBUG(2, "[wrapper] gp_text_style_alloc (%p)", style);

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
