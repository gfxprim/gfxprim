%include "../common.i"
%module(package="gfxprim.gfx") c_gfx

%{
#include "gfx/GP_Gfx.h"
%}

%import ../core/core.i

%include "GP_Gfx.h"

/* Listed in GP_Gfx.h: */
%include "GP_Fill.h"
%include "GP_HLine.h"
%include "GP_VLine.h"
%include "GP_Line.h"
%include "GP_Rect.h"
%include "GP_Triangle.h"
%include "GP_Tetragon.h"
%include "GP_Circle.h"
/* %include "GP_CircleSeg.h"  -- missing symbols */
%include "GP_Ellipse.h"
%include "GP_Arc.h"
%include "GP_Polygon.h"

%include "GP_PutPixelAA.h"
%include "GP_VLineAA.h"
%include "GP_HLineAA.h"
%include "GP_LineAA.h"

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


