%include "../common.i"
%module(package="gfxprim.core") c_core

%{
#include "core/GP_Core.h"
%}

/*
 * Basic types and common methods
 */

%include "GP_Core.h"
%include "GP_Types.h"
%include "GP_Transform.h"
%include "GP_GetSetBits.h"
%include "GP_Transform.h"

/*
 * Make members of GP_DebugMsg structure immutable
 */
%immutable GP_DebugMsg::level;
%immutable GP_DebugMsg::file;
%immutable GP_DebugMsg::fn;
%immutable GP_DebugMsg::line;
%immutable GP_DebugMsg::msg;
%ignore GP_DebugPrint;

%include "GP_Debug.h"

/*
 * Color and pixel types
 */

ERROR_ON_NULL(GP_ColorToColorName);

%include "GP_Color.h"
%include "GP_Pixel.h"
%include "GP_Pixel.gen.h" /* Includes enum GP_PixelType definition */
%include "GP_Convert.h"
%import "GP_Convert.gen.h"
%import "GP_Convert_Scale.gen.h"

%import "GP_FnPerBpp.h"
%import "GP_FnPerBpp.gen.h"

%inline %{
const GP_PixelTypeDescription *GP_PixelTypes_access(GP_PixelType no)
{
        if ((signed)no < 0 || no >= GP_PIXEL_MAX) no = GP_PIXEL_UNKNOWN;
        return &GP_PixelTypes[no];
}
%}

/*
 * GP_Context wrapping
 */

/* Make some members RO */
%immutable GP_Context::w;
%immutable GP_Context::h;
%immutable GP_Context::pixel_type;
%immutable GP_Context::bpp;
%immutable GP_Context::bytes_per_row;
/* Rename "internal" GP_Context */
%rename("_%s") "GP_Context::pixels";
%rename("_%s") "GP_Context::offset";
%rename("_%s") "GP_Context::axes_swap";
%rename("_%s") "GP_Context::x_swap";
%rename("_%s") "GP_Context::y_swap";
%rename("_%s") "GP_Context::bit_endian";
%rename("_%s") "GP_Context::free_pixels";


%feature("autodoc", "Proxy of C GP_Context struct

You can pass this class to wrapped GP_DrawSomething(...) as GP_Context.
All attributes of GP_Context are accessible directly as _attribute
(self._w etc.), but it is reccomended to use redefined properties:

self.w: Context width (transformed)
self.h: Context height (transformed)
self.pixel_type: Context pixel type (number)

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
  GP_Context(GP_Coord w, GP_Coord h, GP_PixelType typeno) {
    return GP_ContextAlloc(w, h, typeno);
  }
};


/* Error handling */
ERROR_ON_NONZERO(GP_ContextResize);
ERROR_ON_NULL(GP_ContextAlloc);
ERROR_ON_NULL(GP_ContextCopy);
ERROR_ON_NULL(GP_ContextConvertAlloc);
ERROR_ON_NULL(GP_SubContextAlloc);

/* Indicate new wrapper-owned GP_Context */
%newobject GP_ContextAlloc;
%newobject GP_ContextCopy;
%newobject GP_ContextConvertAlloc;
%newobject GP_SubContextAlloc;

%include "GP_Context.h"

/*
 * Context manipulation
 */

%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"
%include "GP_Blit.h"

