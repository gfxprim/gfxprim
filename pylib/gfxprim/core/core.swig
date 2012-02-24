%module core_c

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
 * GP_Context wrapping
 */

/* Rename accesors to GP_Context attributes to "_attribute" */
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
/* This nice batch would work in swig 2.0, sigh:
 * %rename("_%s", regexmatch$name="^GP_Context::") ""; */

%rename("Context") "GP_Context";

%feature("autodoc", "Proxy of C GP_Context struct

You can pass this class to wrapped GP_DrawSomething(...) as GP_Context.
All attributes of GP_Context are accessible directly as _attribute
(self._w etc.), but it is reccomended to use redefined properties:

self.w: Context width (transformed)
self.h: Context width (transformed)

Some context-related methods are provided as class members for convenience.

GP_Context memory allocation is handled by gfxprim, deallocation by GP_ContextFree().
The wrapper can be used without owning the GP_Context struct by setting self.this
and self.thisown.") GP_Context;

%extend GP_Context {
  ~GP_Context() {
    GP_DEBUG(2, "[wrapper] GP_ContextFree (%dx%d raw, %dbpp, free_pixels:%d)",
      $self->w, $self->h, $self->bpp, $self->free_pixels);
    GP_ContextFree($self);
  }
};

/* Indicate new wrapper-owned GP_Context */
%newobject GP_ContextAlloc;
%newobject GP_ContextCopy;
%newobject GP_ContextSubContext;
%newobject GP_ContextConvert;

%include "GP_Context.h"


/*
 * Context manipulation
 */

%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"
%include "GP_Blit.h"
