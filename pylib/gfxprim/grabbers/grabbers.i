%include "../common.i"
%module(package="gfxprim.grabbers") c_grabbers

%{
#include "GP_Grabbers.h"
#include "core/GP_Debug.h"
%}

/* %import ../core/core.i */

/*
 * General grabber structure handling
 */
%extend GP_Grabber {
  ~GP_Grabber() {
    GP_DEBUG(2, "[wrapper] GP_GrabberExit (%p)", $self);
    GP_GrabberExit($self);
  }
};

%ignore GP_Grabber::priv;
%ignore GP_Grabber::fd;

%ignore GP_Grabber::Start;
%ignore GP_Grabber::Stop;
%ignore GP_Grabber::Exit;
%ignore GP_Grabber::Poll;

%include "GP_Grabber.h"

/*
 * Particular grabbers.
 */
ERROR_ON_NULL(GP_GrabberV4L2Init);
%newobject GP_GrabberV4L2Init;
%include "GP_V4L2.h"
