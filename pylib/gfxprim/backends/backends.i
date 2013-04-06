%include "../common.i"
%module(package="gfxprim.backends") c_backends

%{
#include "GP_Backend.h"
#include "GP_Backends.h"
#include "core/GP_Debug.h"
%}

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

ERROR_ON_NONZERO(GP_BackendSetCaption);
ERROR_ON_NONZERO(GP_BackendResize);

%include "GP_Backend.h"

/*
 * Particular backends.
 */
ERROR_ON_NULL(GP_BackendVirtualInit);
%newobject GP_BackendVirtualInit;
%include "GP_BackendVirtual.h"

ERROR_ON_NULL(GP_BackendLinuxFBInit);
%newobject GP_BackendLinuxFBInit;
%include "GP_LinuxFB.h"

ERROR_ON_NULL(GP_BackendLinuxSDLInit);
%newobject GP_BackendSDLInit;
%include "GP_SDL.h"

ERROR_ON_NULL(GP_BackendLinuxX11Init);
%newobject GP_BackendX11Init;
%include "GP_X11.h"

/*
 * Backend Init with a typemap for FILE* for the last parameter
 *
 * This is a ugly hack because Python 3 uses its own I/O buffers for file
 * objects.
 * 
 */
%typemap(in) FILE* {
    if ($input != Py_None) {
        int fd = PyObject_AsFileDescriptor($input);

        if (fd < 0) {
            PyErr_SetString(PyExc_TypeError, "$1_name must be a file type.");
            return NULL;
        }
       
       $1 = fdopen(fd, "w");
    }
}

%exception GP_BackendInit {
$action
  
  //HACK: free the FILE*
  free(arg3);
  
  if (result == NULL)
    return PyErr_SetFromErrno(PyExc_OSError);
}

%newobject GP_BackendInit;
%include "GP_BackendInit.h"

