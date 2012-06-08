%include "../common.i"
%module(package="gfxprim.loaders") loaders_c

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

%import ../core/core.i

ERROR_ON_NULL(GP_LoadImage);
ERROR_ON_NONZERO(GP_LoadMetaData);
ERROR_ON_NONZERO(GP_SaveImage);

%include "GP_Loaders.h"

ERROR_ON_NONZERO(GP_OpenJPG);
ERROR_ON_NULL(GP_ReadJPG);
ERROR_ON_NULL(GP_LoadJPG);
ERROR_ON_NONZERO(GP_ReadJPGMetaData);
ERROR_ON_NONZERO(GP_LoadJPGMetaData);
ERROR_ON_NONZERO(GP_SaveJPG);

%include "GP_JPG.h"

ERROR_ON_NONZERO(GP_OpenBMP);
ERROR_ON_NULL(GP_ReadBMP);
ERROR_ON_NULL(GP_LoadBMP);

%include "GP_BMP.h"

ERROR_ON_NONZERO(GP_OpenGIF);
ERROR_ON_NULL(GP_ReadGIF);
ERROR_ON_NULL(GP_LoadGIF);

%include "GP_GIF.h"

ERROR_ON_NONZERO(GP_OpenPNG);
ERROR_ON_NULL(GP_ReadPNG);
ERROR_ON_NULL(GP_LoadPNG);
ERROR_ON_NONZERO(GP_ReadPNGMetaData);
ERROR_ON_NONZERO(GP_LoadPNGMetaData);
ERROR_ON_NONZERO(GP_SavePNG);

%include "GP_PNG.h"

/* TODO: No error checking - legacy GP_RetCode form */
%include "GP_PBM.h"
%include "GP_PGM.h"
%include "GP_PPM.h"


