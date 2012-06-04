/*
 * common SWIG declarations for all modules
 */

#define __attribute__(X)

%include <stdint.i>

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
    int errno0 = errno;
    const int errbuf_len = 128;
    char errbuf[errbuf_len];
    strerror_r(errno0, errbuf, errbuf_len);
    PyErr_Format(PyExc_RuntimeError, "Error in function %s: %s", "$name", errbuf);
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
    int errno0 = errno;
    const int errbuf_len = 128;
    char errbuf[errbuf_len];
    strerror_r(errno0, errbuf, errbuf_len);
    PyErr_Format(PyExc_RuntimeError, "Error in function %s: %s", "$name", errbuf);
    return NULL;
  }
}
%enddef
