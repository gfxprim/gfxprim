/*
 * common SWIG declarations for all modules
 */

#define __attribute__(X)

%include <stdint.i>
%include <exception.i>

%feature("autodoc");

%nodefaultctor;

/*
 * Error handling declarations
 */

/*
 * Wrapped function is executed and the return value is checked.
 * If NULL, errno is checked and an exception raised.
 */

%define ERROR_ON_NULL(funcname)
%exception funcname {
  $action
  if (result == NULL) {
    char errbuf[128];
    char *errmsg = strerror_r(errno, errbuf, sizeof(errbuf));
    PyErr_Format(PyExc_RuntimeError, "Error in function %s: %s", "$name", errmsg);
    return NULL;
  }
}
%enddef

/*
 * Wrapped function is executed and the return value is checked.
 * If non-zero, errno is checked and an exception raised
 */

%define ERROR_ON_NONZERO(funcname)
%exception funcname {
  $action
  if (result != 0) {
    char errbuf[128];
    char *errmsg = strerror_r(errno, errbuf, sizeof(errbuf));
    PyErr_Format(PyExc_RuntimeError, "Error in function %s: %s", "$name", errmsg);
    return NULL;
  }
}
%enddef
