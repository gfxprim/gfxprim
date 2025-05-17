%include "../common.i"
%module(package="gfxprim.core") c_core

%{
#include "core/gp_core.h"
%}

/*
 * Basic types and common methods
 */

%immutable gp_symmetry_names;
%include "gp_core.h"
%include "gp_types.h"
%include "gp_compiler.h"
%include "gp_transform.h"
%include "gp_get_set_bits.h"

/*
 * Make members of gp_debug_msg structure immutable
 */
%immutable gp_debug_msg::level;
%immutable gp_debug_msg::file;
%immutable gp_debug_msg::fn;
%immutable gp_debug_msg::line;
%immutable gp_debug_msg::msg;
%ignore gp_debug_print;

%include "gp_debug.h"

/*
 * Pixel types
 */
%include "gp_pixel.h"
%include "gp_pixel.gen.h"
%include "gp_pixel_alias.h"
%include "gp_convert.h"
%import "gp_convert.gen.h"
%import "gp_convert_scale.gen.h"

%import "gp_pixel_pack.gen.h"

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
%immutable gp_pixmap::bytes_per_row;
%immutable gp_pixmap::gamma;
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

gp_pixmap memory allocation is handled by gfxprim, deallocation by gp_pixmap_free().
The wrapper can be used without owning the gp_pixmap struct by setting self.this
and self.thisown.") gp_pixmap;

%extend gp_pixmap {
  ~gp_pixmap() {
    GP_DEBUG(2, "[wrapper] gp_pixmap_free (%dx%d raw, free_pixels:%d)",
      $self->w, $self->h, $self->free_pixels);
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

%include "gp_pixmap.h"

/*
 * Pixmap manipulation
 */
%include "gp_get_put_pixel.h"
%import "gp_get_put_pixel.gen.h"
%include "gp_write_pixels.gen.h"
%include "gp_blit.h"
%include "gp_fill.h"
