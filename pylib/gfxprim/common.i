/*
 * common SWIG declarations for all modules
 */

#define __attribute__(X)

%include <stdint.i>
%include <exception.i>

%feature("autodoc");

%nodefaultctor;


/*
 * Python callback proxy. Calls a python function passed via self->priv
 */
%{
#include <core/GP_ProgressCallback.h>
#include <core/GP_Debug.h>

struct gp_proxy_params {
        PyObject *callback;
        PyObject *args;
};

int gp_proxy_callback(gp_progress_cb *self)
{
        struct gp_proxy_params *params = self->priv;
        PyObject *res, *args;
        int ret;

        GP_DEBUG(2, "[wrapper] Proxy Callback called");

        if (params->args)
                args = Py_BuildValue("(f, O)", self->percentage, params->args);
        else
                args = Py_BuildValue("(f)", self->percentage);

        res = PyEval_CallObject(params->callback, args);

        /* Parse Error, Interruption, etc. */
        if (res == NULL) {
                PyErr_Print();
                GP_WARN("Error while calling callback, aborting");
                return 1;
        }

        if (PyInt_Check(res)) {
                ret = PyInt_AsLong(res);
        } else {
                GP_WARN("Wrong type returned from callback, aborting");
                return 1;
        }

        Py_DECREF(res);
        Py_DECREF(args);

        return ret;
}
%}

/*
 * Progress callback typemap, package python function into proxy callback structure
 * and passes it to the C function.
 *
 * The python callback object can either be a function or a tuple with a function
 * as a first parameter
 */
%typemap(in) gp_progress_cb *callback(gp_progress_cb callback_proxy,
                                      struct gp_proxy_params proxy_params)
{
        if ($input != Py_None) {
                if (PyTuple_Check($input) && PyTuple_GET_SIZE($input) > 0) {
                        if (!PyCallable_Check(PyTuple_GetItem($input, 0))) {
                                PyErr_SetString(PyExc_TypeError,
                                                "first arg in tuple must be callable");
                                return NULL;
                        }
                        proxy_params.callback = PyTuple_GetItem($input, 0);
                        proxy_params.args = $input;

                } else {
                        if (!PyCallable_Check($input)) {
                                PyErr_SetString(PyExc_TypeError,
                                                "parameter must be callable");
                                return NULL;
                        }

                        proxy_params.callback = $input;
                        proxy_params.args = NULL;
                }
        }

        if ($input && $input != Py_None) {
                callback_proxy.callback = gp_proxy_callback;
                callback_proxy.priv = &proxy_params;
                $1 = &callback_proxy;
        } else {
                $1 = NULL;
        }
}

/*
 * All progress callbacks have default NULL value
 */
%typemap(default) gp_progress_cb *callback {
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
