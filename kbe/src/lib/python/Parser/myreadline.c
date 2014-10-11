
/* Readline interface for tokenizer.c and [raw_]input() in bltinmodule.c.
   By default, or when stdin is not a tty device, we have a super
   simple my_readline function using fgets.
   Optionally, we can use the GNU readline library.
   my_readline() has a different return value from GNU readline():
   - NULL if an interrupt occurred or if an error occurred
   - a malloc'ed empty string if EOF was read
   - a malloc'ed string ending in \n normally
*/

#include "Python.h"
#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif /* MS_WINDOWS */


PyThreadState* _PyOS_ReadlineTState;

#ifdef WITH_THREAD
#include "pythread.h"
static PyThread_type_lock _PyOS_ReadlineLock = NULL;
#endif

int (*PyOS_InputHook)(void) = NULL;

/* This function restarts a fgets() after an EINTR error occurred
   except if PyOS_InterruptOccurred() returns true. */

static int
my_fgets(char *buf, int len, FILE *fp)
{
#ifdef MS_WINDOWS
    HANDLE hInterruptEvent;
#endif
    char *p;
    int err;
    while (1) {
        if (PyOS_InputHook != NULL)
            (void)(PyOS_InputHook)();
        errno = 0;
        clearerr(fp);
        if (_PyVerify_fd(fileno(fp)))
            p = fgets(buf, len, fp);
        else
            p = NULL;
        if (p != NULL)
            return 0; /* No error */
        err = errno;
#ifdef MS_WINDOWS
        /* Ctrl-C anywhere on the line or Ctrl-Z if the only character
           on a line will set ERROR_OPERATION_ABORTED. Under normal
           circumstances Ctrl-C will also have caused the SIGINT handler
           to fire which will have set the event object returned by
           _PyOS_SigintEvent. This signal fires in another thread and
           is not guaranteed to have occurred before this point in the
           code.

           Therefore: check whether the event is set with a small timeout.
           If it is, assume this is a Ctrl-C and reset the event. If it
           isn't set assume that this is a Ctrl-Z on its own and drop
           through to check for EOF.
        */
        if (GetLastError()==ERROR_OPERATION_ABORTED) {
            hInterruptEvent = _PyOS_SigintEvent();
            switch (WaitForSingleObjectEx(hInterruptEvent, 10, FALSE)) {
            case WAIT_OBJECT_0:
                ResetEvent(hInterruptEvent);
                return 1; /* Interrupt */
            case WAIT_FAILED:
                return -2; /* Error */
            }
        }
#endif /* MS_WINDOWS */
        if (feof(fp)) {
            clearerr(fp);
            return -1; /* EOF */
        }
#ifdef EINTR
        if (err == EINTR) {
            int s;
#ifdef WITH_THREAD
            PyEval_RestoreThread(_PyOS_ReadlineTState);
#endif
            s = PyErr_CheckSignals();
#ifdef WITH_THREAD
            PyEval_SaveThread();
#endif
            if (s < 0)
                    return 1;
        /* try again */
            continue;
        }
#endif
        if (PyOS_InterruptOccurred()) {
            return 1; /* Interrupt */
        }
        return -2; /* Error */
    }
    /* NOTREACHED */
}


/* Readline implementation using fgets() */

char *
PyOS_StdioReadline(FILE *sys_stdin, FILE *sys_stdout, const char *prompt)
{
    size_t n;
    char *p, *pr;

    n = 100;
    p = (char *)PyMem_RawMalloc(n);
    if (p == NULL)
        return NULL;

    fflush(sys_stdout);
    if (prompt)
        fprintf(stderr, "%s", prompt);
    fflush(stderr);

    switch (my_fgets(p, (int)n, sys_stdin)) {
    case 0: /* Normal case */
        break;
    case 1: /* Interrupt */
        PyMem_RawFree(p);
        return NULL;
    case -1: /* EOF */
    case -2: /* Error */
    default: /* Shouldn't happen */
        *p = '\0';
        break;
    }
    n = strlen(p);
    while (n > 0 && p[n-1] != '\n') {
        size_t incr = n+2;
        if (incr > INT_MAX) {
            PyMem_RawFree(p);
            PyErr_SetString(PyExc_OverflowError, "input line too long");
            return NULL;
        }
        pr = (char *)PyMem_RawRealloc(p, n + incr);
        if (pr == NULL) {
            PyMem_RawFree(p);
            PyErr_NoMemory();
            return NULL;
        }
        p = pr;
        if (my_fgets(p+n, (int)incr, sys_stdin) != 0)
            break;
        n += strlen(p+n);
    }
    pr = (char *)PyMem_RawRealloc(p, n+1);
    if (pr == NULL) {
        PyMem_RawFree(p);
        PyErr_NoMemory();
        return NULL;
    }
    return pr;
}


/* By initializing this function pointer, systems embedding Python can
   override the readline function.

   Note: Python expects in return a buffer allocated with PyMem_Malloc. */

char *(*PyOS_ReadlineFunctionPointer)(FILE *, FILE *, const char *);


/* Interface used by tokenizer.c and bltinmodule.c */

char *
PyOS_Readline(FILE *sys_stdin, FILE *sys_stdout, const char *prompt)
{
    char *rv, *res;
    size_t len;

    if (_PyOS_ReadlineTState == PyThreadState_GET()) {
        PyErr_SetString(PyExc_RuntimeError,
                        "can't re-enter readline");
        return NULL;
    }


    if (PyOS_ReadlineFunctionPointer == NULL) {
        PyOS_ReadlineFunctionPointer = PyOS_StdioReadline;
    }

#ifdef WITH_THREAD
    if (_PyOS_ReadlineLock == NULL) {
        _PyOS_ReadlineLock = PyThread_allocate_lock();
    }
#endif

    _PyOS_ReadlineTState = PyThreadState_GET();
    Py_BEGIN_ALLOW_THREADS
#ifdef WITH_THREAD
    PyThread_acquire_lock(_PyOS_ReadlineLock, 1);
#endif

    /* This is needed to handle the unlikely case that the
     * interpreter is in interactive mode *and* stdin/out are not
     * a tty.  This can happen, for example if python is run like
     * this: python -i < test1.py
     */
    if (!isatty (fileno (sys_stdin)) || !isatty (fileno (sys_stdout)))
        rv = PyOS_StdioReadline (sys_stdin, sys_stdout, prompt);
    else
        rv = (*PyOS_ReadlineFunctionPointer)(sys_stdin, sys_stdout,
                                             prompt);
    Py_END_ALLOW_THREADS

#ifdef WITH_THREAD
    PyThread_release_lock(_PyOS_ReadlineLock);
#endif

    _PyOS_ReadlineTState = NULL;

    if (rv == NULL)
        return NULL;

    len = strlen(rv) + 1;
    res = PyMem_Malloc(len);
    if (res != NULL)
        memcpy(res, rv, len);
    PyMem_RawFree(rv);

    return res;
}
