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
 * Pixel types
 */
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
 * GP_Pixmap wrapping
 */

/* Make some members RO */
%immutable GP_Pixmap::w;
%immutable GP_Pixmap::h;
%immutable GP_Pixmap::pixel_type;
%immutable GP_Pixmap::bpp;
%immutable GP_Pixmap::bytes_per_row;
/* Rename "internal" GP_Pixmap */
%rename("_%s") "GP_Pixmap::pixels";
%rename("_%s") "GP_Pixmap::offset";
%rename("_%s") "GP_Pixmap::axes_swap";
%rename("_%s") "GP_Pixmap::x_swap";
%rename("_%s") "GP_Pixmap::y_swap";
%rename("_%s") "GP_Pixmap::bit_endian";
%rename("_%s") "GP_Pixmap::free_pixels";

%inline %{
PyObject *GP_PixmapToByteArray(GP_Pixmap *self)
{
        return PyByteArray_FromStringAndSize((char*)self->pixels,
                                             self->bytes_per_row * self->h);
}
%}

%feature("autodoc", "Proxy of C GP_Pixmap struct

You can pass this class to wrapped GP_DrawSomething(...) as GP_Pixmap.
All attributes of GP_Pixmap are accessible directly as _attribute
(self._w etc.), but it is reccomended to use redefined properties:

self.w: Pixmap width (transformed)
self.h: Pixmap height (transformed)
self.pixel_type: Pixmap pixel type (number)

Some pixmap-related methods are provided as class members for convenience.

GP_Pixmap memory allocation is handled by gfxprim, deallocation by GP_PixmapFree().
The wrapper can be used without owning the GP_Pixmap struct by setting self.this
and self.thisown.") GP_Pixmap;

%extend GP_Pixmap {
  ~GP_Pixmap() {
    GP_DEBUG(2, "[wrapper] GP_PixmapFree (%dx%d raw, %dbpp, free_pixels:%d)",
      $self->w, $self->h, $self->bpp, $self->free_pixels);
    GP_PixmapFree($self);
  }
  GP_Pixmap(GP_Coord w, GP_Coord h, GP_PixelType typeno) {
    return GP_PixmapAlloc(w, h, typeno);
  }
};


/* Error handling */
ERROR_ON_NONZERO(GP_PixmapResize);
ERROR_ON_NULL(GP_PixmapAlloc);
ERROR_ON_NULL(GP_PixmapCopy);
ERROR_ON_NULL(GP_PixmapConvertAlloc);
ERROR_ON_NULL(GP_SubPixmapAlloc);

/* Indicate new wrapper-owned GP_Pixmap */
%newobject GP_PixmapAlloc;
%newobject GP_PixmapCopy;
%newobject GP_PixmapConvertAlloc;
%newobject GP_SubPixmapAlloc;

%include "GP_Pixmap.h"

/*
 * Pixmap manipulation
 */
%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"
%include "GP_Blit.h"
%include "GP_Fill.h"
