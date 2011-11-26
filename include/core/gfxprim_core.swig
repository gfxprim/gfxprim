%module gfxprim_core_c

%{
#include "core/GP_Core.h"
%}

#define __attribute__(X) 

%include <stdint.i>

%feature("autodoc");

%nodefaultctor;

/*
 * Basic types and common methods
 */

%include "GP_Common.h"
%include "GP_Core.h"
%include "GP_Debug.h"
%include "GP_Types.h"
%include "GP_Transform.h"
%include "GP_Counter.h"
%include "GP_GetSetBits.h"
%include "GP_Transform.h"
%include "GP_ProgressCallback.h"
%include "GP_RetCode.h"


/*
 * Color and pixel types
 */

%include "GP_Color.h"
%include "GP_Pixel.h"
%include "GP_Pixel.gen.h" /* Includes enum GP_PixelType definition */
%include "GP_Convert.h"
%import "GP_Convert.gen.h"
%import "GP_Convert_Scale.gen.h"

%import "GP_FnPerBpp.h"
%import "GP_FnPerBpp.gen.h"


/*
 * Context manipulation
 */

/* Rename accestors to GP_Context attributes to _attribute */
/* would work in swig 2.0:
 * %rename("_%s", regexmatch$name="^GP_Context::") ""; */
%rename("_%s") "GP_Context::pixels";
%rename("_%s") "GP_Context::bpp";
%rename("_%s") "GP_Context::bytes_per_row";
%rename("_%s") "GP_Context::w";
%rename("_%s") "GP_Context::h";
%rename("_%s") "GP_Context::offset";
%rename("_%s") "GP_Context::pixel_type";
%rename("_%s") "GP_Context::axes_swap";
%rename("_%s") "GP_Context::x_swap";
%rename("_%s") "GP_Context::y_swap";
%rename("_%s") "GP_Context::bit_endian";
%rename("_%s") "GP_Context::free_pixels";
%rename("Context") "GP_Context";

%include "GP_Context.h"

%extend GP_Context {
  ~GP_Context() {
    GP_DEBUG(2, "[wrapper] GP_ContextFree (%dx%d raw, %dbpp, free_pixels:%d)",
      $self->w, $self->h, $self->bpp, $self->free_pixels);
    GP_ContextFree($self);
  }
};

%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"

%include "GP_Blit.h"
%import "GP_Blit.gen.h"
