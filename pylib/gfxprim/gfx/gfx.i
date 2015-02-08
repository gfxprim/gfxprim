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
static GP_Coord *GP_Polygon_unpack_coordinates(PyObject *coords)
{
  unsigned int i, vertex_count;
  GP_Coord *cs;

  GP_ASSERT(PyTuple_Check(coords));
  vertex_count = PyTuple_Size(coords);
  GP_ASSERT(vertex_count % 2 == 0);
  cs = malloc(sizeof(GP_Coord[vertex_count]));
  GP_ASSERT(cs != NULL);
  for (i = 0; i < vertex_count; i++) {
    PyObject *e = PyTuple_GetItem(coords, i); // Borrowed or ?
    GP_ASSERT(PyInt_Check(e));
    cs[i] = PyInt_AsLong(e);
  }
  return cs;
}

void GP_Polygon_wrap(GP_Context *context, PyObject *coords, GP_Pixel pixel)
{
  GP_Coord *cs = GP_Polygon_unpack_coordinates(coords);
  GP_Polygon(context, PyTuple_Size(coords) / 2, cs, pixel);
  free(cs);
}

void GP_FillPolygon_wrap(GP_Context *context, PyObject *coords, GP_Pixel pixel)
{
  GP_Coord *cs = GP_Polygon_unpack_coordinates(coords);
  GP_FillPolygon(context, PyTuple_Size(coords) / 2, cs, pixel);
  free(cs);
}
%}


