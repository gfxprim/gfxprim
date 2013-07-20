%include "../common.i"
%module(package="gfxprim.loaders") c_loaders

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

%ignore GP_Loader;

%import ../core/core.i

ERROR_ON_NULL(GP_LoadImage);
ERROR_ON_NONZERO(GP_LoadMetaData);
ERROR_ON_NONZERO(GP_SaveImage);

%newobject GP_LoadImage;

%include "GP_Loader.h"

ERROR_ON_NONZERO(GP_OpenJPG);
ERROR_ON_NULL(GP_ReadJPG);
ERROR_ON_NULL(GP_LoadJPG);
ERROR_ON_NONZERO(GP_ReadJPGMetaData);
ERROR_ON_NONZERO(GP_LoadJPGMetaData);
ERROR_ON_NONZERO(GP_SaveJPG);

%newobject GP_LoadJPG;

%include "GP_JPG.h"

ERROR_ON_NONZERO(GP_OpenBMP);
ERROR_ON_NULL(GP_ReadBMP);
ERROR_ON_NULL(GP_LoadBMP);

%newobject GP_LoadBMP;

%include "GP_BMP.h"

ERROR_ON_NONZERO(GP_OpenGIF);
ERROR_ON_NULL(GP_ReadGIF);
ERROR_ON_NULL(GP_LoadGIF);

%newobject GP_LoadGIF;

%include "GP_GIF.h"

ERROR_ON_NONZERO(GP_OpenPNG);
ERROR_ON_NULL(GP_ReadPNG);
ERROR_ON_NULL(GP_LoadPNG);
ERROR_ON_NONZERO(GP_ReadPNGMetaData);
ERROR_ON_NONZERO(GP_LoadPNGMetaData);
ERROR_ON_NONZERO(GP_SavePNG);

%newobject GP_LoadPNG;

%include "GP_PNG.h"

ERROR_ON_NULL(GP_LoadPBM);
ERROR_ON_NULL(GP_LoadPGM);
ERROR_ON_NULL(GP_LoadPPM);
ERROR_ON_NULL(GP_LoadPNM);
ERROR_ON_NONZERO(GP_SavePBM);
ERROR_ON_NONZERO(GP_SavePGM);
ERROR_ON_NONZERO(GP_SavePPM);
ERROR_ON_NONZERO(GP_SavePNM);

%newobject GP_LoadPBM;
%newobject GP_LoadPGM;
%newobject GP_LoadPPM;
%newobject GP_LoadPNM;

%include "GP_PNM.h"

ERROR_ON_NULL(GP_LoadTIFF);
ERROR_ON_NONZERO(GP_SaveTIFF);

%newobject GP_LoadTIFF;

%include "GP_TIFF.h"

ERROR_ON_NULL(GP_LoadPSP);

%newobject GP_LoadPSP;

%include "GP_PSP.h"
