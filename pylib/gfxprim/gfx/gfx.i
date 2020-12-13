%include "../common.i"
%module(package="gfxprim.gfx") c_gfx

%{
#include "gfx/gp_gfx.h"
%}

%import ../core/core.i

%include "gp_gfx.h"

/* Listed in GP_Gfx.h: */
%include "gp_fill.h"
%include "gp_hline.h"
%include "gp_vline.h"
%include "gp_line.h"
%include "gp_rect.h"
%include "gp_triangle.h"
%include "gp_tetragon.h"
%include "gp_circle.h"
/* %include "GP_CircleSeg.h"  -- missing symbols */
%include "gp_ellipse.h"
%include "gp_arc.h"
%include "gp_polygon.h"

%inline %{
static gp_coord *gp_polygon_unpack_coordinates(PyObject *coords)
{
  unsigned int i, vertex_count;
  gp_coord *cs;

  GP_ASSERT(PyTuple_Check(coords));
  vertex_count = PyTuple_Size(coords);
  GP_ASSERT(vertex_count % 2 == 0);
  cs = malloc(sizeof(gp_coord[vertex_count]));
  GP_ASSERT(cs != NULL);
  for (i = 0; i < vertex_count; i++) {
    PyObject *e = PyTuple_GetItem(coords, i); // Borrowed or ?
    GP_ASSERT(PyInt_Check(e));
    cs[i] = PyInt_AsLong(e);
  }
  return cs;
}

void gp_polygon_wrap(gp_pixmap *pixmap, PyObject *coords, gp_pixel pixel)
{
  gp_coord *cs = gp_polygon_unpack_coordinates(coords);
  gp_polygon(pixmap, PyTuple_Size(coords) / 2, cs, pixel);
  free(cs);
}

void gp_fill_polygon_wrap(gp_pixmap *pixmap, PyObject *coords, gp_pixel pixel)
{
  gp_coord *cs = gp_polygon_unpack_coordinates(coords);
  gp_fill_polygon(pixmap, PyTuple_Size(coords) / 2, cs, pixel);
  free(cs);
}
%}


