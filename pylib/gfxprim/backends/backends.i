%include "../common.i"
%module(package="gfxprim.backends") c_backends

%{
#include "GP_Backend.h"
#include "GP_Backends.h"
#include <core/gp_debug.h>
%}

%import ../core/core.i


/*
 * General backend structure handling
 */

%extend gp_backend {
  ~gp_backend() {
    GP_DEBUG(2, "[wrapper] gp_backend_exit (%s)",
      $self->name);
    gp_backend_exit($self);
  }
};

%ignore gp_backend::priv;
%ignore gp_backend::fd_list;
%immutable gp_backend::name;
%ignore gp_backend_fd;

ERROR_ON_NONZERO(gp_backend_set_caption);
ERROR_ON_NONZERO(gp_backend_resize);

%include "GP_Backend.h"

/*
 * Particular backends.
 */
ERROR_ON_NULL(gp_backend_virtual_init);
%newobject gp_backend_virtual_init;
%include "GP_BackendVirtual.h"

ERROR_ON_NULL(gp_backend_linux_fb_init);
%newobject GP_BackendLinuxFBInit;
%include "GP_LinuxFB.h"

ERROR_ON_NULL(gp_backend_sdl_init);
%newobject GP_BackendSDLInit;
%include "GP_SDL.h"

ERROR_ON_NULL(gp_backend_x11_init);
%newobject gp_backend_x11_init;
%include "GP_X11.h"

%newobject gp_backend_init;
ERROR_ON_NULL(gp_backend_init);
%include "GP_BackendInit.h"

