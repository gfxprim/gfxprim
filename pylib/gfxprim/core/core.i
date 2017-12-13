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
 * Make members of gp_debug_msg structure immutable
 */
%immutable gp_debug_msg::level;
%immutable gp_debug_msg::file;
%immutable gp_debug_msg::fn;
%immutable gp_debug_msg::line;
%immutable gp_debug_msg::msg;
%ignore gp_debug_print;

%include "GP_Debug.h"

/*
 * Pixel types
 */
%include "GP_Pixel.h"
%include "GP_Pixel.gen.h" /* Includes enum gp_pixel_type definition */
%include "GP_Convert.h"
%import "GP_Convert.gen.h"
%import "GP_Convert_Scale.gen.h"

%import "GP_FnPerBpp.h"
%import "GP_FnPerBpp.gen.h"

%inline %{
const gp_pixel_type_desc *gp_pixel_types_access(gp_pixel_type no)
{
        if ((signed)no < 0 || no >= GP_PIXEL_MAX) no = GP_PIXEL_UNKNOWN;
        return &gp_pixel_types[no];
}
%}

/*
 * gp_pixmap wrapping
 */

/* Make some members RO */
%immutable gp_pixmap::w;
%immutable gp_pixmap::h;
%immutable gp_pixmap::pixel_type;
%immutable gp_pixmap::bpp;
%immutable gp_pixmap::bytes_per_row;
/* Rename "internal" gp_pixmap */
%rename("_%s") "gp_pixmap::pixels";
%rename("_%s") "gp_pixmap::offset";
%rename("_%s") "gp_pixmap::axes_swap";
%rename("_%s") "gp_pixmap::x_swap";
%rename("_%s") "gp_pixmap::y_swap";
%rename("_%s") "gp_pixmap::bit_endian";
%rename("_%s") "gp_pixmap::free_pixels";

%inline %{
PyObject *gp_pixmap_to_byte_array(gp_pixmap *self)
{
        return PyByteArray_FromStringAndSize((char*)self->pixels,
                                             self->bytes_per_row * self->h);
}
%}

%feature("autodoc", "Proxy of C gp_pixmap struct

You can pass this class to wrapped gp_draw_something(...) as gp_pixmap.
All attributes of gp_pixmap are accessible directly as _attribute
(self._w etc.), but it is reccomended to use redefined properties:

self.w: Pixmap width (transformed)
self.h: Pixmap height (transformed)
self.pixel_type: Pixmap pixel type (number)

Some pixmap-related methods are provided as class members for convenience.

gp_pixmap memory allocation is handled by gfxprim, deallocation by gp_pixmapFree().
The wrapper can be used without owning the gp_pixmap struct by setting self.this
and self.thisown.") gp_pixmap;

%extend gp_pixmap {
  ~gp_pixmap() {
    GP_DEBUG(2, "[wrapper] gp_pixmapFree (%dx%d raw, %dbpp, free_pixels:%d)",
      $self->w, $self->h, $self->bpp, $self->free_pixels);
    gp_pixmap_free($self);
  }
  gp_pixmap(gp_coord w, gp_coord h, gp_pixel_type typeno) {
    return gp_pixmap_alloc(w, h, typeno);
  }
};


/* Error handling */
ERROR_ON_NONZERO(gp_pixmap_resize);
ERROR_ON_NULL(gp_pixmap_alloc);
ERROR_ON_NULL(gp_pixmap_copy);
ERROR_ON_NULL(gp_pixmap_convert_alloc);
ERROR_ON_NULL(gp_sub_pixmap_alloc);

/* Indicate new wrapper-owned gp_pixmap */
%newobject gp_pixmap_alloc;
%newobject gp_pixmap_copy;
%newobject gp_pixmap_convert_alloc;
%newobject gp_sub_pixmap_alloc;

%include "GP_Pixmap.h"

/*
 * Pixmap manipulation
 */
%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"
%include "GP_Blit.h"
%include "GP_Fill.h"
