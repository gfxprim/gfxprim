%module gfxprim_core_c

%{
#include "core/GP_Core.h"
%}

#define __attribute__(X) 

%include <stdint.i>

%nodefaultctor;

/*
 * Basic types and common methods
 */

%include "GP_Common.h"
%include "GP_Core.h"
%include "GP_Debug.h"
%include "GP_Types.h"
%include "GP_Transform.h"
%include "GP_Counter.h"
%include "GP_GetSetBits.h"
%include "GP_Transform.h"
%include "GP_ProgressCallback.h"
%include "GP_RetCode.h"


/*
 * Color and pixel types
 */

%include "GP_Color.h"
%include "GP_Pixel.h"
%include "GP_Pixel.gen.h" /* Includes enum GP_PixelType definition */
%include "GP_Convert.h"
%import "GP_Convert.gen.h"
%import "GP_Convert_Scale.gen.h"

%import "GP_FnPerBpp.h"
%import "GP_FnPerBpp.gen.h"


/*
 * Context manipulation
 */

%include "GP_Context.h"

%include "GP_GetPutPixel.h"
%import "GP_GetPutPixel.gen.h"
%include "GP_WritePixel.h"

%include "GP_Blit.h"
%import "GP_Blit.gen.h"
