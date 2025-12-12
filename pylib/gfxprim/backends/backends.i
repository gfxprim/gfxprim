%include "../common.i"
%module(package="gfxprim.backends") c_backends

%{
#include "gp_backend.h"
#include "gp_backends.h"
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
%ignore gp_backend::task_queue;

ERROR_ON_NONZERO(gp_backend_set_caption);
ERROR_ON_NONZERO(gp_backend_render_stopped);

%include "gp_backend.h"

/*
 * Particular backends.
 */
ERROR_ON_NULL(gp_backend_virtual_init);
%newobject gp_backend_virtual_init;
%include "gp_backend_virtual.h"

ERROR_ON_NULL(gp_backend_linux_fb_init);
%newobject GP_BackendLinuxFBInit;
%include "gp_linux_fb.h"

ERROR_ON_NULL(gp_backend_sdl_init);
%newobject GP_BackendSDLInit;
%include "gp_sdl.h"

ERROR_ON_NULL(gp_backend_x11_init);
%newobject gp_backend_x11_init;
%include "gp_x11.h"

%newobject gp_backend_init;
ERROR_ON_NULL(gp_backend_init);
%include "gp_backend_init.h"

