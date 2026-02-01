%include "../common.i"
%module(package="gfxprim.grabbers") c_grabbers

%{
#include <grabbers/gp_grabbers.h>
#include <core/gp_debug.h>
%}

/* %import ../core/core.i */

/*
 * General grabber structure handling
 */
%extend gp_grabber {
  ~gp_grabber() {
    GP_DEBUG(2, "[wrapper] gp_grabberExit (%p)", $self);
    gp_grabber_exit($self);
  }
};

%ignore gp_grabber::priv;
%ignore gp_grabber::fd;

%ignore gp_grabber::start;
%ignore gp_grabber::stop;
%ignore gp_grabber::exit;
%ignore gp_grabber::poll;

/*
 * Particular grabbers.
 */
ERROR_ON_NULL(gp_grabber_v4l2_init);
%newobject gp_grabber_v4l2_init;
%include "gp_v4l2.h"
