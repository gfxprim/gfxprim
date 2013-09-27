/*
 * common SWIG declarations for all modules
 */

#define __attribute__(X)

%include <stdint.i>
%include <exception.i>

%feature("autodoc");

%nodefaultctor;

/*
 * All progress callbacks have default NULL value
 */
%typemap(default) GP_ProgressCallback *callback {
        $1 = NULL;
}

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
  if (result == NULL)
    return PyErr_SetFromErrno(PyExc_OSError);
}
%enddef

/*
 * Wrapped function is executed and the return value is checked.
 * If non-zero, errno is checked and an exception raised
 */

%define ERROR_ON_NONZERO(funcname)
%exception funcname {
  $action
  if (result != 0)
    return PyErr_SetFromErrno(PyExc_OSError);
}
%enddef
