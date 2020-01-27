%include "../common.i"
%module(package="gfxprim.loaders") c_loaders

%{
#include "core/gp_core.h"
#include "loaders/GP_Loaders.h"
%}

%ignore gp_loader;

%import ../core/core.i

/* TODO IO from fd */

%define LOADER_FUNC(FMT)
%newobject gp_load ## FMT;
ERROR_ON_NULL(gp_load ## FMT);
%newobject gp_read ## FMT;
ERROR_ON_NULL(gp_read ## FMT);
%enddef

ERROR_ON_NULL(gp_load_image);
ERROR_ON_NONZERO(gp_read_image);
ERROR_ON_NONZERO(gp_save_image);

%newobject gp_load_image;

%include "GP_Loader.h"

LOADER_FUNC(jpg);
ERROR_ON_NONZERO(gp_save_jpg);

LOADER_FUNC(bmp);
ERROR_ON_NONZERO(gp_save_bmp);

LOADER_FUNC(gif);

LOADER_FUNC(png);
ERROR_ON_NONZERO(gp_save_png);

LOADER_FUNC(pbm);
LOADER_FUNC(pgm);
LOADER_FUNC(ppm);
LOADER_FUNC(pnm);
ERROR_ON_NONZERO(gp_save_pbm);
ERROR_ON_NONZERO(gp_save_pgm);
ERROR_ON_NONZERO(gp_save_ppm);
ERROR_ON_NONZERO(gp_save_pnm);

LOADER_FUNC(tiff);
ERROR_ON_NONZERO(gp_save_tiff);

LOADER_FUNC(jp2);
LOADER_FUNC(pcx);
LOADER_FUNC(psp);
LOADER_FUNC(psd);

%include "GP_Loaders.gen.h"
