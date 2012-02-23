%module loaders_c

%{
#include "core/GP_Core.h"
#include "loaders/GP_Loaders.h"
%}

#define __attribute__(X) 

%import ../core/core.swig

%include <stdint.i>

%nodefaultctor;

%include "GP_Loaders.h"

%{
GP_Context *GP_LoadImage_Wrap(const char *src_path)
{
	GP_Context *c = NULL;
	if (GP_LoadImage(src_path, &c, NULL) != 0)
		return NULL;
	return c;
}
%}

GP_Context *GP_LoadImage_Wrap(const char *src_path);

%include "GP_JPG.h"
%include "GP_PBM.h"
%include "GP_PGM.h"
%include "GP_PPM.h"
%include "GP_PNG.h"

