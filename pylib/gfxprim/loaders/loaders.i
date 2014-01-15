%include "../common.i"
%module(package="gfxprim.loaders") c_loaders

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

%ignore GP_Loader;

%import ../core/core.i

/* TODO IO from fd */

%define LOADER_FUNC(FMT)
%newobject GP_Load ## FMT;
ERROR_ON_NULL(GP_Load ## FMT);
%newobject GP_Read ## FMT;
ERROR_ON_NULL(GP_Read ## FMT);
%enddef

ERROR_ON_NULL(GP_LoadImage);
ERROR_ON_NONZERO(GP_ReadImage);
ERROR_ON_NONZERO(GP_SaveImage);

%newobject GP_LoadImage;

%include "GP_Loader.h"

LOADER_FUNC(JPG);
ERROR_ON_NONZERO(GP_SaveJPG);

%include "GP_JPG.h"

LOADER_FUNC(BMP);
ERROR_ON_NONZERO(GP_SaveBMP);

%include "GP_BMP.h"

LOADER_FUNC(GIF);

%include "GP_GIF.h"

LOADER_FUNC(PNG);
ERROR_ON_NONZERO(GP_SavePNG);

%include "GP_PNG.h"

LOADER_FUNC(PBM);
LOADER_FUNC(PGM);
LOADER_FUNC(PPM);
LOADER_FUNC(PNM);
ERROR_ON_NONZERO(GP_SavePBM);
ERROR_ON_NONZERO(GP_SavePGM);
ERROR_ON_NONZERO(GP_SavePPM);
ERROR_ON_NONZERO(GP_SavePNM);

%include "GP_PNM.h"

LOADER_FUNC(TIFF);
ERROR_ON_NONZERO(GP_SaveTIFF);

%include "GP_TIFF.h"

LOADER_FUNC(PSP);

%include "GP_PSP.h"

LOADER_FUNC(JP2);

%include "GP_JP2.h"
