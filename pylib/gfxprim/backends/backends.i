%module(package="gfxprim.backends") backends_c

%{
#include "GP_Backend.h"
#include "GP_Backends.h"
#include "core/GP_Debug.h"
%}

#define __attribute__(X)
%include <stdint.i>

%feature("autodoc");

%import ../core/core.i


/*
 * General backend structure handling
 */

%extend GP_Backend {
  ~GP_Backend() {
    GP_DEBUG(2, "[wrapper] GP_BackendExit (%s)",
      $self->name);
    GP_BackendExit($self);
  }
};

%ignore GP_Backend::priv;
%ignore GP_Backend::fd_list;
%immutable GP_Backend::name;
%ignore GP_BackendFD;

%include "GP_Backend.h"

/*
 * Particular backends. We need to list every header separately.
 */
/* Overall backend init header */
%newobject GP_BackendInit;
%include "GP_BackendInit.h"

%newobject GP_BackendLinuxFBInit;
%include "GP_LinuxFB.h"

%newobject GP_BackendSDLInit;
%include "GP_SDL.h"

%newobject GP_BackendX11Init;
%include "GP_X11.h"
