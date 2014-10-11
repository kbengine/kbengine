
/* System module */

/*
Various bits of information used by the interpreter are collected in
module 'sys'.
Function member:
- exit(sts): raise SystemExit
Data members:
- stdin, stdout, stderr: standard file objects
- modules: the table of modules (dictionary)
- path: module search path (list of strings)
- argv: script arguments (list of strings)
- ps1, ps2: optional primary and secondary prompts (strings)
*/

#include "Python.h"
#include "code.h"
#include "frameobject.h"
#include "pythread.h"

#include "osdefs.h"

#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* MS_WINDOWS */

#ifdef MS_COREDLL
extern void *PyWin_DLLhModule;
/* A string loaded from the DLL at startup: */
extern const char *PyWin_DLLVersionString;
#endif

#ifdef HAVE_LANGINFO_H
#include <locale.h>
#include <langinfo.h>
#endif

_Py_IDENTIFIER(_);
_Py_IDENTIFIER(__sizeof__);
_Py_IDENTIFIER(buffer);
_Py_IDENTIFIER(builtins);
_Py_IDENTIFIER(encoding);
_Py_IDENTIFIER(path);
_Py_IDENTIFIER(stdout);
_Py_IDENTIFIER(stderr);
_Py_IDENTIFIER(write);

PyObject *
_PySys_GetObjectId(_Py_Identifier *key)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *sd = tstate->interp->sysdict;
    if (sd == NULL)
        return NULL;
    return _PyDict_GetItemId(sd, key);
}

PyObject *
PySys_GetObject(const char *name)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *sd = tstate->interp->sysdict;
    if (sd == NULL)
        return NULL;
    return PyDict_GetItemString(sd, name);
}

int
_PySys_SetObjectId(_Py_Identifier *key, PyObject *v)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *sd = tstate->interp->sysdict;
    if (v == NULL) {
        if (_PyDict_GetItemId(sd, key) == NULL)
            return 0;
        else
            return _PyDict_DelItemId(sd, key);
    }
    else
        return _PyDict_SetItemId(sd, key, v);
}

int
PySys_SetObject(const char *name, PyObject *v)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *sd = tstate->interp->sysdict;
    if (v == NULL) {
        if (PyDict_GetItemString(sd, name) == NULL)
            return 0;
        else
            return PyDict_DelItemString(sd, name);
    }
    else
        return PyDict_SetItemString(sd, name, v);
}

/* Write repr(o) to sys.stdout using sys.stdout.encoding and 'backslashreplace'
   error handler. If sys.stdout has a buffer attribute, use
   sys.stdout.buffer.write(encoded), otherwise redecode the string and use
   sys.stdout.write(redecoded).

   Helper function for sys_displayhook(). */
static int
sys_displayhook_unencodable(PyObject *outf, PyObject *o)
{
    PyObject *stdout_encoding = NULL;
    PyObject *encoded, *escaped_str, *repr_str, *buffer, *result;
    char *stdout_encoding_str;
    int ret;

    stdout_encoding = _PyObject_GetAttrId(outf, &PyId_encoding);
    if (stdout_encoding == NULL)
        goto error;
    stdout_encoding_str = _PyUnicode_AsString(stdout_encoding);
    if (stdout_encoding_str == NULL)
        goto error;

    repr_str = PyObject_Repr(o);
    if (repr_str == NULL)
        goto error;
    encoded = PyUnicode_AsEncodedString(repr_str,
                                        stdout_encoding_str,
                                        "backslashreplace");
    Py_DECREF(repr_str);
    if (encoded == NULL)
        goto error;

    buffer = _PyObject_GetAttrId(outf, &PyId_buffer);
    if (buffer) {
        result = _PyObject_CallMethodId(buffer, &PyId_write, "(O)", encoded);
        Py_DECREF(buffer);
        Py_DECREF(encoded);
        if (result == NULL)
            goto error;
        Py_DECREF(result);
    }
    else {
        PyErr_Clear();
        escaped_str = PyUnicode_FromEncodedObject(encoded,
                                                  stdout_encoding_str,
                                                  "strict");
        Py_DECREF(encoded);
        if (PyFile_WriteObject(escaped_str, outf, Py_PRINT_RAW) != 0) {
            Py_DECREF(escaped_str);
            goto error;
        }
        Py_DECREF(escaped_str);
    }
    ret = 0;
    goto finally;

error:
    ret = -1;
finally:
    Py_XDECREF(stdout_encoding);
    return ret;
}

static PyObject *
sys_displayhook(PyObject *self, PyObject *o)
{
    PyObject *outf;
    PyInterpreterState *interp = PyThreadState_GET()->interp;
    PyObject *modules = interp->modules;
    PyObject *builtins;
    static PyObject *newline = NULL;
    int err;

    builtins = _PyDict_GetItemId(modules, &PyId_builtins);
    if (builtins == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "lost builtins module");
        return NULL;
    }

    /* Print value except if None */
    /* After printing, also assign to '_' */
    /* Before, set '_' to None to avoid recursion */
    if (o == Py_None) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    if (_PyObject_SetAttrId(builtins, &PyId__, Py_None) != 0)
        return NULL;
    outf = _PySys_GetObjectId(&PyId_stdout);
    if (outf == NULL || outf == Py_None) {
        PyErr_SetString(PyExc_RuntimeError, "lost sys.stdout");
        return NULL;
    }
    if (PyFile_WriteObject(o, outf, 0) != 0) {
        if (PyErr_ExceptionMatches(PyExc_UnicodeEncodeError)) {
            /* repr(o) is not encodable to sys.stdout.encoding with
             * sys.stdout.errors error handler (which is probably 'strict') */
            PyErr_Clear();
            err = sys_displayhook_unencodable(outf, o);
            if (err)
                return NULL;
        }
        else {
            return NULL;
        }
    }
    if (newline == NULL) {
        newline = PyUnicode_FromString("\n");
        if (newline == NULL)
            return NULL;
    }
    if (PyFile_WriteObject(newline, outf, Py_PRINT_RAW) != 0)
        return NULL;
    if (_PyObject_SetAttrId(builtins, &PyId__, o) != 0)
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(displayhook_doc,
"displayhook(object) -> None\n"
"\n"
"Print an object to sys.stdout and also save it in builtins._\n"
);

static PyObject *
sys_excepthook(PyObject* self, PyObject* args)
{
    PyObject *exc, *value, *tb;
    if (!PyArg_UnpackTuple(args, "excepthook", 3, 3, &exc, &value, &tb))
        return NULL;
    PyErr_Display(exc, value, tb);
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(excepthook_doc,
"excepthook(exctype, value, traceback) -> None\n"
"\n"
"Handle an exception by displaying it with a traceback on sys.stderr.\n"
);

static PyObject *
sys_exc_info(PyObject *self, PyObject *noargs)
{
    PyThreadState *tstate;
    tstate = PyThreadState_GET();
    return Py_BuildValue(
        "(OOO)",
        tstate->exc_type != NULL ? tstate->exc_type : Py_None,
        tstate->exc_value != NULL ? tstate->exc_value : Py_None,
        tstate->exc_traceback != NULL ?
            tstate->exc_traceback : Py_None);
}

PyDoc_STRVAR(exc_info_doc,
"exc_info() -> (type, value, traceback)\n\
\n\
Return information about the most recent exception caught by an except\n\
clause in the current stack frame or in an older stack frame."
);

static PyObject *
sys_exit(PyObject *self, PyObject *args)
{
    PyObject *exit_code = 0;
    if (!PyArg_UnpackTuple(args, "exit", 0, 1, &exit_code))
        return NULL;
    /* Raise SystemExit so callers may catch it or clean up. */
    PyErr_SetObject(PyExc_SystemExit, exit_code);
    return NULL;
}

PyDoc_STRVAR(exit_doc,
"exit([status])\n\
\n\
Exit the interpreter by raising SystemExit(status).\n\
If the status is omitted or None, it defaults to zero (i.e., success).\n\
If the status is an integer, it will be used as the system exit status.\n\
If it is another kind of object, it will be printed and the system\n\
exit status will be one (i.e., failure)."
);


static PyObject *
sys_getdefaultencoding(PyObject *self)
{
    return PyUnicode_FromString(PyUnicode_GetDefaultEncoding());
}

PyDoc_STRVAR(getdefaultencoding_doc,
"getdefaultencoding() -> string\n\
\n\
Return the current default string encoding used by the Unicode \n\
implementation."
);

static PyObject *
sys_getfilesystemencoding(PyObject *self)
{
    if (Py_FileSystemDefaultEncoding)
        return PyUnicode_FromString(Py_FileSystemDefaultEncoding);
    PyErr_SetString(PyExc_RuntimeError,
                    "filesystem encoding is not initialized");
    return NULL;
}

PyDoc_STRVAR(getfilesystemencoding_doc,
"getfilesystemencoding() -> string\n\
\n\
Return the encoding used to convert Unicode filenames in\n\
operating system filenames."
);

static PyObject *
sys_intern(PyObject *self, PyObject *args)
{
    PyObject *s;
    if (!PyArg_ParseTuple(args, "U:intern", &s))
        return NULL;
    if (PyUnicode_CheckExact(s)) {
        Py_INCREF(s);
        PyUnicode_InternInPlace(&s);
        return s;
    }
    else {
        PyErr_Format(PyExc_TypeError,
                        "can't intern %.400s", s->ob_type->tp_name);
        return NULL;
    }
}

PyDoc_STRVAR(intern_doc,
"intern(string) -> string\n\
\n\
``Intern'' the given string.  This enters the string in the (global)\n\
table of interned strings whose purpose is to speed up dictionary lookups.\n\
Return the string itself or the previously interned string object with the\n\
same value.");


/*
 * Cached interned string objects used for calling the profile and
 * trace functions.  Initialized by trace_init().
 */
static PyObject *whatstrings[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int
trace_init(void)
{
    static char *whatnames[7] = {"call", "exception", "line", "return",
                                    "c_call", "c_exception", "c_return"};
    PyObject *name;
    int i;
    for (i = 0; i < 7; ++i) {
        if (whatstrings[i] == NULL) {
            name = PyUnicode_InternFromString(whatnames[i]);
            if (name == NULL)
                return -1;
            whatstrings[i] = name;
        }
    }
    return 0;
}


static PyObject *
call_trampoline(PyObject* callback,
                PyFrameObject *frame, int what, PyObject *arg)
{
    PyObject *args;
    PyObject *whatstr;
    PyObject *result;

    args = PyTuple_New(3);
    if (args == NULL)
        return NULL;
    if (PyFrame_FastToLocalsWithError(frame) < 0)
        return NULL;

    Py_INCREF(frame);
    whatstr = whatstrings[what];
    Py_INCREF(whatstr);
    if (arg == NULL)
        arg = Py_None;
    Py_INCREF(arg);
    PyTuple_SET_ITEM(args, 0, (PyObject *)frame);
    PyTuple_SET_ITEM(args, 1, whatstr);
    PyTuple_SET_ITEM(args, 2, arg);

    /* call the Python-level function */
    result = PyEval_CallObject(callback, args);
    PyFrame_LocalsToFast(frame, 1);
    if (result == NULL)
        PyTraceBack_Here(frame);

    /* cleanup */
    Py_DECREF(args);
    return result;
}

static int
profile_trampoline(PyObject *self, PyFrameObject *frame,
                   int what, PyObject *arg)
{
    PyObject *result;

    if (arg == NULL)
        arg = Py_None;
    result = call_trampoline(self, frame, what, arg);
    if (result == NULL) {
        PyEval_SetProfile(NULL, NULL);
        return -1;
    }
    Py_DECREF(result);
    return 0;
}

static int
trace_trampoline(PyObject *self, PyFrameObject *frame,
                 int what, PyObject *arg)
{
    PyObject *callback;
    PyObject *result;

    if (what == PyTrace_CALL)
        callback = self;
    else
        callback = frame->f_trace;
    if (callback == NULL)
        return 0;
    result = call_trampoline(callback, frame, what, arg);
    if (result == NULL) {
        PyEval_SetTrace(NULL, NULL);
        Py_CLEAR(frame->f_trace);
        return -1;
    }
    if (result != Py_None) {
        PyObject *temp = frame->f_trace;
        frame->f_trace = NULL;
        Py_XDECREF(temp);
        frame->f_trace = result;
    }
    else {
        Py_DECREF(result);
    }
    return 0;
}

static PyObject *
sys_settrace(PyObject *self, PyObject *args)
{
    if (trace_init() == -1)
        return NULL;
    if (args == Py_None)
        PyEval_SetTrace(NULL, NULL);
    else
        PyEval_SetTrace(trace_trampoline, args);
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(settrace_doc,
"settrace(function)\n\
\n\
Set the global debug tracing function.  It will be called on each\n\
function call.  See the debugger chapter in the library manual."
);

static PyObject *
sys_gettrace(PyObject *self, PyObject *args)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *temp = tstate->c_traceobj;

    if (temp == NULL)
        temp = Py_None;
    Py_INCREF(temp);
    return temp;
}

PyDoc_STRVAR(gettrace_doc,
"gettrace()\n\
\n\
Return the global debug tracing function set with sys.settrace.\n\
See the debugger chapter in the library manual."
);

static PyObject *
sys_setprofile(PyObject *self, PyObject *args)
{
    if (trace_init() == -1)
        return NULL;
    if (args == Py_None)
        PyEval_SetProfile(NULL, NULL);
    else
        PyEval_SetProfile(profile_trampoline, args);
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(setprofile_doc,
"setprofile(function)\n\
\n\
Set the profiling function.  It will be called on each function call\n\
and return.  See the profiler chapter in the library manual."
);

static PyObject *
sys_getprofile(PyObject *self, PyObject *args)
{
    PyThreadState *tstate = PyThreadState_GET();
    PyObject *temp = tstate->c_profileobj;

    if (temp == NULL)
        temp = Py_None;
    Py_INCREF(temp);
    return temp;
}

PyDoc_STRVAR(getprofile_doc,
"getprofile()\n\
\n\
Return the profiling function set with sys.setprofile.\n\
See the profiler chapter in the library manual."
);

static int _check_interval = 100;

static PyObject *
sys_setcheckinterval(PyObject *self, PyObject *args)
{
    if (PyErr_WarnEx(PyExc_DeprecationWarning,
                     "sys.getcheckinterval() and sys.setcheckinterval() "
                     "are deprecated.  Use sys.setswitchinterval() "
                     "instead.", 1) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i:setcheckinterval", &_check_interval))
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(setcheckinterval_doc,
"setcheckinterval(n)\n\
\n\
Tell the Python interpreter to check for asynchronous events every\n\
n instructions.  This also affects how often thread switches occur."
);

static PyObject *
sys_getcheckinterval(PyObject *self, PyObject *args)
{
    if (PyErr_WarnEx(PyExc_DeprecationWarning,
                     "sys.getcheckinterval() and sys.setcheckinterval() "
                     "are deprecated.  Use sys.getswitchinterval() "
                     "instead.", 1) < 0)
        return NULL;
    return PyLong_FromLong(_check_interval);
}

PyDoc_STRVAR(getcheckinterval_doc,
"getcheckinterval() -> current check interval; see setcheckinterval()."
);

#ifdef WITH_THREAD
static PyObject *
sys_setswitchinterval(PyObject *self, PyObject *args)
{
    double d;
    if (!PyArg_ParseTuple(args, "d:setswitchinterval", &d))
        return NULL;
    if (d <= 0.0) {
        PyErr_SetString(PyExc_ValueError,
                        "switch interval must be strictly positive");
        return NULL;
    }
    _PyEval_SetSwitchInterval((unsigned long) (1e6 * d));
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(setswitchinterval_doc,
"setswitchinterval(n)\n\
\n\
Set the ideal thread switching delay inside the Python interpreter\n\
The actual frequency of switching threads can be lower if the\n\
interpreter executes long sequences of uninterruptible code\n\
(this is implementation-specific and workload-dependent).\n\
\n\
The parameter must represent the desired switching delay in seconds\n\
A typical value is 0.005 (5 milliseconds)."
);

static PyObject *
sys_getswitchinterval(PyObject *self, PyObject *args)
{
    return PyFloat_FromDouble(1e-6 * _PyEval_GetSwitchInterval());
}

PyDoc_STRVAR(getswitchinterval_doc,
"getswitchinterval() -> current thread switch interval; see setswitchinterval()."
);

#endif /* WITH_THREAD */

#ifdef WITH_TSC
static PyObject *
sys_settscdump(PyObject *self, PyObject *args)
{
    int bool;
    PyThreadState *tstate = PyThreadState_Get();

    if (!PyArg_ParseTuple(args, "i:settscdump", &bool))
        return NULL;
    if (bool)
        tstate->interp->tscdump = 1;
    else
        tstate->interp->tscdump = 0;
    Py_INCREF(Py_None);
    return Py_None;

}

PyDoc_STRVAR(settscdump_doc,
"settscdump(bool)\n\
\n\
If true, tell the Python interpreter to dump VM measurements to\n\
stderr.  If false, turn off dump.  The measurements are based on the\n\
processor's time-stamp counter."
);
#endif /* TSC */

static PyObject *
sys_setrecursionlimit(PyObject *self, PyObject *args)
{
    int new_limit;
    if (!PyArg_ParseTuple(args, "i:setrecursionlimit", &new_limit))
        return NULL;
    if (new_limit <= 0) {
        PyErr_SetString(PyExc_ValueError,
                        "recursion limit must be positive");
        return NULL;
    }
    Py_SetRecursionLimit(new_limit);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyTypeObject Hash_InfoType;

PyDoc_STRVAR(hash_info_doc,
"hash_info\n\
\n\
A struct sequence providing parameters used for computing\n\
hashes. The attributes are read only.");

static PyStructSequence_Field hash_info_fields[] = {
    {"width", "width of the type used for hashing, in bits"},
    {"modulus", "prime number giving the modulus on which the hash "
                "function is based"},
    {"inf", "value to be used for hash of a positive infinity"},
    {"nan", "value to be used for hash of a nan"},
    {"imag", "multiplier used for the imaginary part of a complex number"},
    {"algorithm", "name of the algorithm for hashing of str, bytes and "
                  "memoryviews"},
    {"hash_bits", "internal output size of hash algorithm"},
    {"seed_bits", "seed size of hash algorithm"},
    {"cutoff", "small string optimization cutoff"},
    {NULL, NULL}
};

static PyStructSequence_Desc hash_info_desc = {
    "sys.hash_info",
    hash_info_doc,
    hash_info_fields,
    9,
};

static PyObject *
get_hash_info(void)
{
    PyObject *hash_info;
    int field = 0;
    PyHash_FuncDef *hashfunc;
    hash_info = PyStructSequence_New(&Hash_InfoType);
    if (hash_info == NULL)
        return NULL;
    hashfunc = PyHash_GetFuncDef();
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(8*sizeof(Py_hash_t)));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromSsize_t(_PyHASH_MODULUS));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(_PyHASH_INF));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(_PyHASH_NAN));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(_PyHASH_IMAG));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyUnicode_FromString(hashfunc->name));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(hashfunc->hash_bits));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(hashfunc->seed_bits));
    PyStructSequence_SET_ITEM(hash_info, field++,
                              PyLong_FromLong(Py_HASH_CUTOFF));
    if (PyErr_Occurred()) {
        Py_CLEAR(hash_info);
        return NULL;
    }
    return hash_info;
}


PyDoc_STRVAR(setrecursionlimit_doc,
"setrecursionlimit(n)\n\
\n\
Set the maximum depth of the Python interpreter stack to n.  This\n\
limit prevents infinite recursion from causing an overflow of the C\n\
stack and crashing Python.  The highest possible limit is platform-\n\
dependent."
);

static PyObject *
sys_getrecursionlimit(PyObject *self)
{
    return PyLong_FromLong(Py_GetRecursionLimit());
}

PyDoc_STRVAR(getrecursionlimit_doc,
"getrecursionlimit()\n\
\n\
Return the current value of the recursion limit, the maximum depth\n\
of the Python interpreter stack.  This limit prevents infinite\n\
recursion from causing an overflow of the C stack and crashing Python."
);

#ifdef MS_WINDOWS
PyDoc_STRVAR(getwindowsversion_doc,
"getwindowsversion()\n\
\n\
Return information about the running version of Windows as a named tuple.\n\
The members are named: major, minor, build, platform, service_pack,\n\
service_pack_major, service_pack_minor, suite_mask, and product_type. For\n\
backward compatibility, only the first 5 items are available by indexing.\n\
All elements are numbers, except service_pack which is a string. Platform\n\
may be 0 for win32s, 1 for Windows 9x/ME, 2 for Windows NT/2000/XP/Vista/7,\n\
3 for Windows CE. Product_type may be 1 for a workstation, 2 for a domain\n\
controller, 3 for a server."
);

static PyTypeObject WindowsVersionType = {0, 0, 0, 0, 0, 0};

static PyStructSequence_Field windows_version_fields[] = {
    {"major", "Major version number"},
    {"minor", "Minor version number"},
    {"build", "Build number"},
    {"platform", "Operating system platform"},
    {"service_pack", "Latest Service Pack installed on the system"},
    {"service_pack_major", "Service Pack major version number"},
    {"service_pack_minor", "Service Pack minor version number"},
    {"suite_mask", "Bit mask identifying available product suites"},
    {"product_type", "System product type"},
    {0}
};

static PyStructSequence_Desc windows_version_desc = {
    "sys.getwindowsversion",  /* name */
    getwindowsversion_doc,    /* doc */
    windows_version_fields,   /* fields */
    5                         /* For backward compatibility,
                                 only the first 5 items are accessible
                                 via indexing, the rest are name only */
};

static PyObject *
sys_getwindowsversion(PyObject *self)
{
    PyObject *version;
    int pos = 0;
    OSVERSIONINFOEX ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    if (!GetVersionEx((OSVERSIONINFO*) &ver))
        return PyErr_SetFromWindowsErr(0);

    version = PyStructSequence_New(&WindowsVersionType);
    if (version == NULL)
        return NULL;

    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.dwMajorVersion));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.dwMinorVersion));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.dwBuildNumber));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.dwPlatformId));
    PyStructSequence_SET_ITEM(version, pos++, PyUnicode_FromString(ver.szCSDVersion));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.wServicePackMajor));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.wServicePackMinor));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.wSuiteMask));
    PyStructSequence_SET_ITEM(version, pos++, PyLong_FromLong(ver.wProductType));

    if (PyErr_Occurred()) {
        Py_DECREF(version);
        return NULL;
    }
    return version;
}

#endif /* MS_WINDOWS */

#ifdef HAVE_DLOPEN
static PyObject *
sys_setdlopenflags(PyObject *self, PyObject *args)
{
    int new_val;
    PyThreadState *tstate = PyThreadState_GET();
    if (!PyArg_ParseTuple(args, "i:setdlopenflags", &new_val))
        return NULL;
    if (!tstate)
        return NULL;
    tstate->interp->dlopenflags = new_val;
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(setdlopenflags_doc,
"setdlopenflags(n) -> None\n\
\n\
Set the flags used by the interpreter for dlopen calls, such as when the\n\
interpreter loads extension modules.  Among other things, this will enable\n\
a lazy resolving of symbols when importing a module, if called as\n\
sys.setdlopenflags(0).  To share symbols across extension modules, call as\n\
sys.setdlopenflags(os.RTLD_GLOBAL).  Symbolic names for the flag modules\n\
can be found in the os module (RTLD_xxx constants, e.g. os.RTLD_LAZY).");

static PyObject *
sys_getdlopenflags(PyObject *self, PyObject *args)
{
    PyThreadState *tstate = PyThreadState_GET();
    if (!tstate)
        return NULL;
    return PyLong_FromLong(tstate->interp->dlopenflags);
}

PyDoc_STRVAR(getdlopenflags_doc,
"getdlopenflags() -> int\n\
\n\
Return the current value of the flags that are used for dlopen calls.\n\
The flag constants are defined in the os module.");

#endif  /* HAVE_DLOPEN */

#ifdef USE_MALLOPT
/* Link with -lmalloc (or -lmpc) on an SGI */
#include <malloc.h>

static PyObject *
sys_mdebug(PyObject *self, PyObject *args)
{
    int flag;
    if (!PyArg_ParseTuple(args, "i:mdebug", &flag))
        return NULL;
    mallopt(M_DEBUG, flag);
    Py_INCREF(Py_None);
    return Py_None;
}
#endif /* USE_MALLOPT */

static PyObject *
sys_getsizeof(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *res = NULL;
    static PyObject *gc_head_size = NULL;
    static char *kwlist[] = {"object", "default", 0};
    PyObject *o, *dflt = NULL;
    PyObject *method;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O:getsizeof",
                                     kwlist, &o, &dflt))
        return NULL;

    /* Initialize static variable for GC head size */
    if (gc_head_size == NULL) {
        gc_head_size = PyLong_FromSsize_t(sizeof(PyGC_Head));
        if (gc_head_size == NULL)
            return NULL;
    }

    /* Make sure the type is initialized. float gets initialized late */
    if (PyType_Ready(Py_TYPE(o)) < 0)
        return NULL;

    method = _PyObject_LookupSpecial(o, &PyId___sizeof__);
    if (method == NULL) {
        if (!PyErr_Occurred())
            PyErr_Format(PyExc_TypeError,
                         "Type %.100s doesn't define __sizeof__",
                         Py_TYPE(o)->tp_name);
    }
    else {
        res = PyObject_CallFunctionObjArgs(method, NULL);
        Py_DECREF(method);
    }

    /* Has a default value been given */
    if ((res == NULL) && (dflt != NULL) &&
        PyErr_ExceptionMatches(PyExc_TypeError))
    {
        PyErr_Clear();
        Py_INCREF(dflt);
        return dflt;
    }
    else if (res == NULL)
        return res;

    /* add gc_head size */
    if (PyObject_IS_GC(o)) {
        PyObject *tmp = res;
        res = PyNumber_Add(tmp, gc_head_size);
        Py_DECREF(tmp);
    }
    return res;
}

PyDoc_STRVAR(getsizeof_doc,
"getsizeof(object, default) -> int\n\
\n\
Return the size of object in bytes.");

static PyObject *
sys_getrefcount(PyObject *self, PyObject *arg)
{
    return PyLong_FromSsize_t(arg->ob_refcnt);
}

#ifdef Py_REF_DEBUG
static PyObject *
sys_gettotalrefcount(PyObject *self)
{
    return PyLong_FromSsize_t(_Py_GetRefTotal());
}
#endif /* Py_REF_DEBUG */

PyDoc_STRVAR(getrefcount_doc,
"getrefcount(object) -> integer\n\
\n\
Return the reference count of object.  The count returned is generally\n\
one higher than you might expect, because it includes the (temporary)\n\
reference as an argument to getrefcount()."
);

static PyObject *
sys_getallocatedblocks(PyObject *self)
{
    return PyLong_FromSsize_t(_Py_GetAllocatedBlocks());
}

PyDoc_STRVAR(getallocatedblocks_doc,
"getallocatedblocks() -> integer\n\
\n\
Return the number of memory blocks currently allocated, regardless of their\n\
size."
);

#ifdef COUNT_ALLOCS
static PyObject *
sys_getcounts(PyObject *self)
{
    extern PyObject *get_counts(void);

    return get_counts();
}
#endif

PyDoc_STRVAR(getframe_doc,
"_getframe([depth]) -> frameobject\n\
\n\
Return a frame object from the call stack.  If optional integer depth is\n\
given, return the frame object that many calls below the top of the stack.\n\
If that is deeper than the call stack, ValueError is raised.  The default\n\
for depth is zero, returning the frame at the top of the call stack.\n\
\n\
This function should be used for internal and specialized\n\
purposes only."
);

static PyObject *
sys_getframe(PyObject *self, PyObject *args)
{
    PyFrameObject *f = PyThreadState_GET()->frame;
    int depth = -1;

    if (!PyArg_ParseTuple(args, "|i:_getframe", &depth))
        return NULL;

    while (depth > 0 && f != NULL) {
        f = f->f_back;
        --depth;
    }
    if (f == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "call stack is not deep enough");
        return NULL;
    }
    Py_INCREF(f);
    return (PyObject*)f;
}

PyDoc_STRVAR(current_frames_doc,
"_current_frames() -> dictionary\n\
\n\
Return a dictionary mapping each current thread T's thread id to T's\n\
current stack frame.\n\
\n\
This function should be used for specialized purposes only."
);

static PyObject *
sys_current_frames(PyObject *self, PyObject *noargs)
{
    return _PyThread_CurrentFrames();
}

PyDoc_STRVAR(call_tracing_doc,
"call_tracing(func, args) -> object\n\
\n\
Call func(*args), while tracing is enabled.  The tracing state is\n\
saved, and restored afterwards.  This is intended to be called from\n\
a debugger from a checkpoint, to recursively debug some other code."
);

static PyObject *
sys_call_tracing(PyObject *self, PyObject *args)
{
    PyObject *func, *funcargs;
    if (!PyArg_ParseTuple(args, "OO!:call_tracing", &func, &PyTuple_Type, &funcargs))
        return NULL;
    return _PyEval_CallTracing(func, funcargs);
}

PyDoc_STRVAR(callstats_doc,
"callstats() -> tuple of integers\n\
\n\
Return a tuple of function call statistics, if CALL_PROFILE was defined\n\
when Python was built.  Otherwise, return None.\n\
\n\
When enabled, this function returns detailed, implementation-specific\n\
details about the number of function calls executed. The return value is\n\
a 11-tuple where the entries in the tuple are counts of:\n\
0. all function calls\n\
1. calls to PyFunction_Type objects\n\
2. PyFunction calls that do not create an argument tuple\n\
3. PyFunction calls that do not create an argument tuple\n\
   and bypass PyEval_EvalCodeEx()\n\
4. PyMethod calls\n\
5. PyMethod calls on bound methods\n\
6. PyType calls\n\
7. PyCFunction calls\n\
8. generator calls\n\
9. All other calls\n\
10. Number of stack pops performed by call_function()"
);

#ifdef __cplusplus
extern "C" {
#endif

static PyObject *
sys_debugmallocstats(PyObject *self, PyObject *args)
{
#ifdef WITH_PYMALLOC
    _PyObject_DebugMallocStats(stderr);
    fputc('\n', stderr);
#endif
    _PyObject_DebugTypeStats(stderr);

    Py_RETURN_NONE;
}
PyDoc_STRVAR(debugmallocstats_doc,
"_debugmallocstats()\n\
\n\
Print summary info to stderr about the state of\n\
pymalloc's structures.\n\
\n\
In Py_DEBUG mode, also perform some expensive internal consistency\n\
checks.\n\
");

#ifdef Py_TRACE_REFS
/* Defined in objects.c because it uses static globals if that file */
extern PyObject *_Py_GetObjects(PyObject *, PyObject *);
#endif

#ifdef DYNAMIC_EXECUTION_PROFILE
/* Defined in ceval.c because it uses static globals if that file */
extern PyObject *_Py_GetDXProfile(PyObject *,  PyObject *);
#endif

#ifdef __cplusplus
}
#endif

static PyObject *
sys_clear_type_cache(PyObject* self, PyObject* args)
{
    PyType_ClearCache();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(sys_clear_type_cache__doc__,
"_clear_type_cache() -> None\n\
Clear the internal type lookup cache.");


static PyMethodDef sys_methods[] = {
    /* Might as well keep this in alphabetic order */
    {"callstats", (PyCFunction)PyEval_GetCallStats, METH_NOARGS,
     callstats_doc},
    {"_clear_type_cache",       sys_clear_type_cache,     METH_NOARGS,
     sys_clear_type_cache__doc__},
    {"_current_frames", sys_current_frames, METH_NOARGS,
     current_frames_doc},
    {"displayhook",     sys_displayhook, METH_O, displayhook_doc},
    {"exc_info",        sys_exc_info, METH_NOARGS, exc_info_doc},
    {"excepthook",      sys_excepthook, METH_VARARGS, excepthook_doc},
    {"exit",            sys_exit, METH_VARARGS, exit_doc},
    {"getdefaultencoding", (PyCFunction)sys_getdefaultencoding,
     METH_NOARGS, getdefaultencoding_doc},
#ifdef HAVE_DLOPEN
    {"getdlopenflags", (PyCFunction)sys_getdlopenflags, METH_NOARGS,
     getdlopenflags_doc},
#endif
    {"getallocatedblocks", (PyCFunction)sys_getallocatedblocks, METH_NOARGS,
      getallocatedblocks_doc},
#ifdef COUNT_ALLOCS
    {"getcounts",       (PyCFunction)sys_getcounts, METH_NOARGS},
#endif
#ifdef DYNAMIC_EXECUTION_PROFILE
    {"getdxp",          _Py_GetDXProfile, METH_VARARGS},
#endif
    {"getfilesystemencoding", (PyCFunction)sys_getfilesystemencoding,
     METH_NOARGS, getfilesystemencoding_doc},
#ifdef Py_TRACE_REFS
    {"getobjects",      _Py_GetObjects, METH_VARARGS},
#endif
#ifdef Py_REF_DEBUG
    {"gettotalrefcount", (PyCFunction)sys_gettotalrefcount, METH_NOARGS},
#endif
    {"getrefcount",     (PyCFunction)sys_getrefcount, METH_O, getrefcount_doc},
    {"getrecursionlimit", (PyCFunction)sys_getrecursionlimit, METH_NOARGS,
     getrecursionlimit_doc},
    {"getsizeof",   (PyCFunction)sys_getsizeof,
     METH_VARARGS | METH_KEYWORDS, getsizeof_doc},
    {"_getframe", sys_getframe, METH_VARARGS, getframe_doc},
#ifdef MS_WINDOWS
    {"getwindowsversion", (PyCFunction)sys_getwindowsversion, METH_NOARGS,
     getwindowsversion_doc},
#endif /* MS_WINDOWS */
    {"intern",          sys_intern,     METH_VARARGS, intern_doc},
#ifdef USE_MALLOPT
    {"mdebug",          sys_mdebug, METH_VARARGS},
#endif
    {"setcheckinterval",        sys_setcheckinterval, METH_VARARGS,
     setcheckinterval_doc},
    {"getcheckinterval",        sys_getcheckinterval, METH_NOARGS,
     getcheckinterval_doc},
#ifdef WITH_THREAD
    {"setswitchinterval",       sys_setswitchinterval, METH_VARARGS,
     setswitchinterval_doc},
    {"getswitchinterval",       sys_getswitchinterval, METH_NOARGS,
     getswitchinterval_doc},
#endif
#ifdef HAVE_DLOPEN
    {"setdlopenflags", sys_setdlopenflags, METH_VARARGS,
     setdlopenflags_doc},
#endif
    {"setprofile",      sys_setprofile, METH_O, setprofile_doc},
    {"getprofile",      sys_getprofile, METH_NOARGS, getprofile_doc},
    {"setrecursionlimit", sys_setrecursionlimit, METH_VARARGS,
     setrecursionlimit_doc},
#ifdef WITH_TSC
    {"settscdump", sys_settscdump, METH_VARARGS, settscdump_doc},
#endif
    {"settrace",        sys_settrace, METH_O, settrace_doc},
    {"gettrace",        sys_gettrace, METH_NOARGS, gettrace_doc},
    {"call_tracing", sys_call_tracing, METH_VARARGS, call_tracing_doc},
    {"_debugmallocstats", sys_debugmallocstats, METH_NOARGS,
     debugmallocstats_doc},
    {NULL,              NULL}           /* sentinel */
};

static PyObject *
list_builtin_module_names(void)
{
    PyObject *list = PyList_New(0);
    int i;
    if (list == NULL)
        return NULL;
    for (i = 0; PyImport_Inittab[i].name != NULL; i++) {
        PyObject *name = PyUnicode_FromString(
            PyImport_Inittab[i].name);
        if (name == NULL)
            break;
        PyList_Append(list, name);
        Py_DECREF(name);
    }
    if (PyList_Sort(list) != 0) {
        Py_DECREF(list);
        list = NULL;
    }
    if (list) {
        PyObject *v = PyList_AsTuple(list);
        Py_DECREF(list);
        list = v;
    }
    return list;
}

static PyObject *warnoptions = NULL;

void
PySys_ResetWarnOptions(void)
{
    if (warnoptions == NULL || !PyList_Check(warnoptions))
        return;
    PyList_SetSlice(warnoptions, 0, PyList_GET_SIZE(warnoptions), NULL);
}

void
PySys_AddWarnOptionUnicode(PyObject *unicode)
{
    if (warnoptions == NULL || !PyList_Check(warnoptions)) {
        Py_XDECREF(warnoptions);
        warnoptions = PyList_New(0);
        if (warnoptions == NULL)
            return;
    }
    PyList_Append(warnoptions, unicode);
}

void
PySys_AddWarnOption(const wchar_t *s)
{
    PyObject *unicode;
    unicode = PyUnicode_FromWideChar(s, -1);
    if (unicode == NULL)
        return;
    PySys_AddWarnOptionUnicode(unicode);
    Py_DECREF(unicode);
}

int
PySys_HasWarnOptions(void)
{
    return (warnoptions != NULL && (PyList_Size(warnoptions) > 0)) ? 1 : 0;
}

static PyObject *xoptions = NULL;

static PyObject *
get_xoptions(void)
{
    if (xoptions == NULL || !PyDict_Check(xoptions)) {
        Py_XDECREF(xoptions);
        xoptions = PyDict_New();
    }
    return xoptions;
}

void
PySys_AddXOption(const wchar_t *s)
{
    PyObject *opts;
    PyObject *name = NULL, *value = NULL;
    const wchar_t *name_end;

    opts = get_xoptions();
    if (opts == NULL)
        goto error;

    name_end = wcschr(s, L'=');
    if (!name_end) {
        name = PyUnicode_FromWideChar(s, -1);
        value = Py_True;
        Py_INCREF(value);
    }
    else {
        name = PyUnicode_FromWideChar(s, name_end - s);
        value = PyUnicode_FromWideChar(name_end + 1, -1);
    }
    if (name == NULL || value == NULL)
        goto error;
    PyDict_SetItem(opts, name, value);
    Py_DECREF(name);
    Py_DECREF(value);
    return;

error:
    Py_XDECREF(name);
    Py_XDECREF(value);
    /* No return value, therefore clear error state if possible */
    if (_Py_atomic_load_relaxed(&_PyThreadState_Current))
        PyErr_Clear();
}

PyObject *
PySys_GetXOptions(void)
{
    return get_xoptions();
}

/* XXX This doc string is too long to be a single string literal in VC++ 5.0.
   Two literals concatenated works just fine.  If you have a K&R compiler
   or other abomination that however *does* understand longer strings,
   get rid of the !!! comment in the middle and the quotes that surround it. */
PyDoc_VAR(sys_doc) =
PyDoc_STR(
"This module provides access to some objects used or maintained by the\n\
interpreter and to functions that interact strongly with the interpreter.\n\
\n\
Dynamic objects:\n\
\n\
argv -- command line arguments; argv[0] is the script pathname if known\n\
path -- module search path; path[0] is the script directory, else ''\n\
modules -- dictionary of loaded modules\n\
\n\
displayhook -- called to show results in an interactive session\n\
excepthook -- called to handle any uncaught exception other than SystemExit\n\
  To customize printing in an interactive session or to install a custom\n\
  top-level exception handler, assign other functions to replace these.\n\
\n\
stdin -- standard input file object; used by input()\n\
stdout -- standard output file object; used by print()\n\
stderr -- standard error object; used for error messages\n\
  By assigning other file objects (or objects that behave like files)\n\
  to these, it is possible to redirect all of the interpreter's I/O.\n\
\n\
last_type -- type of last uncaught exception\n\
last_value -- value of last uncaught exception\n\
last_traceback -- traceback of last uncaught exception\n\
  These three are only available in an interactive session after a\n\
  traceback has been printed.\n\
"
)
/* concatenating string here */
PyDoc_STR(
"\n\
Static objects:\n\
\n\
builtin_module_names -- tuple of module names built into this interpreter\n\
copyright -- copyright notice pertaining to this interpreter\n\
exec_prefix -- prefix used to find the machine-specific Python library\n\
executable -- absolute path of the executable binary of the Python interpreter\n\
float_info -- a struct sequence with information about the float implementation.\n\
float_repr_style -- string indicating the style of repr() output for floats\n\
hash_info -- a struct sequence with information about the hash algorithm.\n\
hexversion -- version information encoded as a single integer\n\
implementation -- Python implementation information.\n\
int_info -- a struct sequence with information about the int implementation.\n\
maxsize -- the largest supported length of containers.\n\
maxunicode -- the value of the largest Unicode codepoint\n\
platform -- platform identifier\n\
prefix -- prefix used to find the Python library\n\
thread_info -- a struct sequence with information about the thread implementation.\n\
version -- the version of this interpreter as a string\n\
version_info -- version information as a named tuple\n\
"
)
#ifdef MS_WINDOWS
/* concatenating string here */
PyDoc_STR(
"dllhandle -- [Windows only] integer handle of the Python DLL\n\
winver -- [Windows only] version number of the Python DLL\n\
"
)
#endif /* MS_WINDOWS */
PyDoc_STR(
"__stdin__ -- the original stdin; don't touch!\n\
__stdout__ -- the original stdout; don't touch!\n\
__stderr__ -- the original stderr; don't touch!\n\
__displayhook__ -- the original displayhook; don't touch!\n\
__excepthook__ -- the original excepthook; don't touch!\n\
\n\
Functions:\n\
\n\
displayhook() -- print an object to the screen, and save it in builtins._\n\
excepthook() -- print an exception and its traceback to sys.stderr\n\
exc_info() -- return thread-safe information about the current exception\n\
exit() -- exit the interpreter by raising SystemExit\n\
getdlopenflags() -- returns flags to be used for dlopen() calls\n\
getprofile() -- get the global profiling function\n\
getrefcount() -- return the reference count for an object (plus one :-)\n\
getrecursionlimit() -- return the max recursion depth for the interpreter\n\
getsizeof() -- return the size of an object in bytes\n\
gettrace() -- get the global debug tracing function\n\
setcheckinterval() -- control how often the interpreter checks for events\n\
setdlopenflags() -- set the flags to be used for dlopen() calls\n\
setprofile() -- set the global profiling function\n\
setrecursionlimit() -- set the max recursion depth for the interpreter\n\
settrace() -- set the global debug tracing function\n\
"
)
/* end of sys_doc */ ;


PyDoc_STRVAR(flags__doc__,
"sys.flags\n\
\n\
Flags provided through command line arguments or environment vars.");

static PyTypeObject FlagsType;

static PyStructSequence_Field flags_fields[] = {
    {"debug",                   "-d"},
    {"inspect",                 "-i"},
    {"interactive",             "-i"},
    {"optimize",                "-O or -OO"},
    {"dont_write_bytecode",     "-B"},
    {"no_user_site",            "-s"},
    {"no_site",                 "-S"},
    {"ignore_environment",      "-E"},
    {"verbose",                 "-v"},
    /* {"unbuffered",                   "-u"}, */
    /* {"skip_first",                   "-x"}, */
    {"bytes_warning",           "-b"},
    {"quiet",                   "-q"},
    {"hash_randomization",      "-R"},
    {"isolated",                "-I"},
    {0}
};

static PyStructSequence_Desc flags_desc = {
    "sys.flags",        /* name */
    flags__doc__,       /* doc */
    flags_fields,       /* fields */
    13
};

static PyObject*
make_flags(void)
{
    int pos = 0;
    PyObject *seq;

    seq = PyStructSequence_New(&FlagsType);
    if (seq == NULL)
        return NULL;

#define SetFlag(flag) \
    PyStructSequence_SET_ITEM(seq, pos++, PyLong_FromLong(flag))

    SetFlag(Py_DebugFlag);
    SetFlag(Py_InspectFlag);
    SetFlag(Py_InteractiveFlag);
    SetFlag(Py_OptimizeFlag);
    SetFlag(Py_DontWriteBytecodeFlag);
    SetFlag(Py_NoUserSiteDirectory);
    SetFlag(Py_NoSiteFlag);
    SetFlag(Py_IgnoreEnvironmentFlag);
    SetFlag(Py_VerboseFlag);
    /* SetFlag(saw_unbuffered_flag); */
    /* SetFlag(skipfirstline); */
    SetFlag(Py_BytesWarningFlag);
    SetFlag(Py_QuietFlag);
    SetFlag(Py_HashRandomizationFlag);
    SetFlag(Py_IsolatedFlag);
#undef SetFlag

    if (PyErr_Occurred()) {
        Py_DECREF(seq);
        return NULL;
    }
    return seq;
}

PyDoc_STRVAR(version_info__doc__,
"sys.version_info\n\
\n\
Version information as a named tuple.");

static PyTypeObject VersionInfoType;

static PyStructSequence_Field version_info_fields[] = {
    {"major", "Major release number"},
    {"minor", "Minor release number"},
    {"micro", "Patch release number"},
    {"releaselevel", "'alpha', 'beta', 'candidate', or 'release'"},
    {"serial", "Serial release number"},
    {0}
};

static PyStructSequence_Desc version_info_desc = {
    "sys.version_info",     /* name */
    version_info__doc__,    /* doc */
    version_info_fields,    /* fields */
    5
};

static PyObject *
make_version_info(void)
{
    PyObject *version_info;
    char *s;
    int pos = 0;

    version_info = PyStructSequence_New(&VersionInfoType);
    if (version_info == NULL) {
        return NULL;
    }

    /*
     * These release level checks are mutually exclusive and cover
     * the field, so don't get too fancy with the pre-processor!
     */
#if PY_RELEASE_LEVEL == PY_RELEASE_LEVEL_ALPHA
    s = "alpha";
#elif PY_RELEASE_LEVEL == PY_RELEASE_LEVEL_BETA
    s = "beta";
#elif PY_RELEASE_LEVEL == PY_RELEASE_LEVEL_GAMMA
    s = "candidate";
#elif PY_RELEASE_LEVEL == PY_RELEASE_LEVEL_FINAL
    s = "final";
#endif

#define SetIntItem(flag) \
    PyStructSequence_SET_ITEM(version_info, pos++, PyLong_FromLong(flag))
#define SetStrItem(flag) \
    PyStructSequence_SET_ITEM(version_info, pos++, PyUnicode_FromString(flag))

    SetIntItem(PY_MAJOR_VERSION);
    SetIntItem(PY_MINOR_VERSION);
    SetIntItem(PY_MICRO_VERSION);
    SetStrItem(s);
    SetIntItem(PY_RELEASE_SERIAL);
#undef SetIntItem
#undef SetStrItem

    if (PyErr_Occurred()) {
        Py_CLEAR(version_info);
        return NULL;
    }
    return version_info;
}

/* sys.implementation values */
#define NAME "cpython"
const char *_PySys_ImplName = NAME;
#define QUOTE(arg) #arg
#define STRIFY(name) QUOTE(name)
#define MAJOR STRIFY(PY_MAJOR_VERSION)
#define MINOR STRIFY(PY_MINOR_VERSION)
#define TAG NAME "-" MAJOR MINOR;
const char *_PySys_ImplCacheTag = TAG;
#undef NAME
#undef QUOTE
#undef STRIFY
#undef MAJOR
#undef MINOR
#undef TAG

static PyObject *
make_impl_info(PyObject *version_info)
{
    int res;
    PyObject *impl_info, *value, *ns;

    impl_info = PyDict_New();
    if (impl_info == NULL)
        return NULL;

    /* populate the dict */

    value = PyUnicode_FromString(_PySys_ImplName);
    if (value == NULL)
        goto error;
    res = PyDict_SetItemString(impl_info, "name", value);
    Py_DECREF(value);
    if (res < 0)
        goto error;

    value = PyUnicode_FromString(_PySys_ImplCacheTag);
    if (value == NULL)
        goto error;
    res = PyDict_SetItemString(impl_info, "cache_tag", value);
    Py_DECREF(value);
    if (res < 0)
        goto error;

    res = PyDict_SetItemString(impl_info, "version", version_info);
    if (res < 0)
        goto error;

    value = PyLong_FromLong(PY_VERSION_HEX);
    if (value == NULL)
        goto error;
    res = PyDict_SetItemString(impl_info, "hexversion", value);
    Py_DECREF(value);
    if (res < 0)
        goto error;

    /* dict ready */

    ns = _PyNamespace_New(impl_info);
    Py_DECREF(impl_info);
    return ns;

error:
    Py_CLEAR(impl_info);
    return NULL;
}

static struct PyModuleDef sysmodule = {
    PyModuleDef_HEAD_INIT,
    "sys",
    sys_doc,
    -1, /* multiple "initialization" just copies the module dict. */
    sys_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyObject *
_PySys_Init(void)
{
    PyObject *m, *sysdict, *version_info;
    int res;

    m = PyModule_Create(&sysmodule);
    if (m == NULL)
        return NULL;
    sysdict = PyModule_GetDict(m);
#define SET_SYS_FROM_STRING_BORROW(key, value)             \
    do {                                                   \
        PyObject *v = (value);                             \
        if (v == NULL)                                     \
            return NULL;                                   \
        res = PyDict_SetItemString(sysdict, key, v);       \
        if (res < 0) {                                     \
            return NULL;                                   \
        }                                                  \
    } while (0)
#define SET_SYS_FROM_STRING(key, value)                    \
    do {                                                   \
        PyObject *v = (value);                             \
        if (v == NULL)                                     \
            return NULL;                                   \
        res = PyDict_SetItemString(sysdict, key, v);       \
        Py_DECREF(v);                                      \
        if (res < 0) {                                     \
            return NULL;                                   \
        }                                                  \
    } while (0)

    /* Check that stdin is not a directory
    Using shell redirection, you can redirect stdin to a directory,
    crashing the Python interpreter. Catch this common mistake here
    and output a useful error message. Note that under MS Windows,
    the shell already prevents that. */
#if !defined(MS_WINDOWS)
    {
        struct stat sb;
        if (fstat(fileno(stdin), &sb) == 0 &&
            S_ISDIR(sb.st_mode)) {
            /* There's nothing more we can do. */
            /* Py_FatalError() will core dump, so just exit. */
            PySys_WriteStderr("Python error: <stdin> is a directory, cannot continue\n");
            exit(EXIT_FAILURE);
        }
    }
#endif

    /* stdin/stdout/stderr are now set by pythonrun.c */

    SET_SYS_FROM_STRING_BORROW("__displayhook__",
                               PyDict_GetItemString(sysdict, "displayhook"));
    SET_SYS_FROM_STRING_BORROW("__excepthook__",
                               PyDict_GetItemString(sysdict, "excepthook"));
    SET_SYS_FROM_STRING("version",
                         PyUnicode_FromString(Py_GetVersion()));
    SET_SYS_FROM_STRING("hexversion",
                         PyLong_FromLong(PY_VERSION_HEX));
    SET_SYS_FROM_STRING("_mercurial",
                        Py_BuildValue("(szz)", "CPython", _Py_hgidentifier(),
                                      _Py_hgversion()));
    SET_SYS_FROM_STRING("dont_write_bytecode",
                         PyBool_FromLong(Py_DontWriteBytecodeFlag));
    SET_SYS_FROM_STRING("api_version",
                        PyLong_FromLong(PYTHON_API_VERSION));
    SET_SYS_FROM_STRING("copyright",
                        PyUnicode_FromString(Py_GetCopyright()));
    SET_SYS_FROM_STRING("platform",
                        PyUnicode_FromString(Py_GetPlatform()));
    SET_SYS_FROM_STRING("executable",
                        PyUnicode_FromWideChar(
                               Py_GetProgramFullPath(), -1));
    SET_SYS_FROM_STRING("prefix",
                        PyUnicode_FromWideChar(Py_GetPrefix(), -1));
    SET_SYS_FROM_STRING("exec_prefix",
                        PyUnicode_FromWideChar(Py_GetExecPrefix(), -1));
    SET_SYS_FROM_STRING("base_prefix",
                        PyUnicode_FromWideChar(Py_GetPrefix(), -1));
    SET_SYS_FROM_STRING("base_exec_prefix",
                        PyUnicode_FromWideChar(Py_GetExecPrefix(), -1));
    SET_SYS_FROM_STRING("maxsize",
                        PyLong_FromSsize_t(PY_SSIZE_T_MAX));
    SET_SYS_FROM_STRING("float_info",
                        PyFloat_GetInfo());
    SET_SYS_FROM_STRING("int_info",
                        PyLong_GetInfo());
    /* initialize hash_info */
    if (Hash_InfoType.tp_name == NULL) {
        if (PyStructSequence_InitType2(&Hash_InfoType, &hash_info_desc) < 0)
            return NULL;
    }
    SET_SYS_FROM_STRING("hash_info",
                        get_hash_info());
    SET_SYS_FROM_STRING("maxunicode",
                        PyLong_FromLong(0x10FFFF));
    SET_SYS_FROM_STRING("builtin_module_names",
                        list_builtin_module_names());
#if PY_BIG_ENDIAN
    SET_SYS_FROM_STRING("byteorder",
                        PyUnicode_FromString("big"));
#else
    SET_SYS_FROM_STRING("byteorder",
                        PyUnicode_FromString("little"));
#endif

#ifdef MS_COREDLL
    SET_SYS_FROM_STRING("dllhandle",
                        PyLong_FromVoidPtr(PyWin_DLLhModule));
    SET_SYS_FROM_STRING("winver",
                        PyUnicode_FromString(PyWin_DLLVersionString));
#endif
#ifdef ABIFLAGS
    SET_SYS_FROM_STRING("abiflags",
                        PyUnicode_FromString(ABIFLAGS));
#endif
    if (warnoptions == NULL) {
        warnoptions = PyList_New(0);
        if (warnoptions == NULL)
            return NULL;
    }
    else {
        Py_INCREF(warnoptions);
    }
    SET_SYS_FROM_STRING_BORROW("warnoptions", warnoptions);

    SET_SYS_FROM_STRING_BORROW("_xoptions", get_xoptions());

    /* version_info */
    if (VersionInfoType.tp_name == NULL) {
        if (PyStructSequence_InitType2(&VersionInfoType,
                                       &version_info_desc) < 0)
            return NULL;
    }
    version_info = make_version_info();
    SET_SYS_FROM_STRING("version_info", version_info);
    /* prevent user from creating new instances */
    VersionInfoType.tp_init = NULL;
    VersionInfoType.tp_new = NULL;
    res = PyDict_DelItemString(VersionInfoType.tp_dict, "__new__");
    if (res < 0 && PyErr_ExceptionMatches(PyExc_KeyError))
        PyErr_Clear();

    /* implementation */
    SET_SYS_FROM_STRING("implementation", make_impl_info(version_info));

    /* flags */
    if (FlagsType.tp_name == 0) {
        if (PyStructSequence_InitType2(&FlagsType, &flags_desc) < 0)
            return NULL;
    }
    SET_SYS_FROM_STRING("flags", make_flags());
    /* prevent user from creating new instances */
    FlagsType.tp_init = NULL;
    FlagsType.tp_new = NULL;
    res = PyDict_DelItemString(FlagsType.tp_dict, "__new__");
    if (res < 0 && PyErr_ExceptionMatches(PyExc_KeyError))
        PyErr_Clear();

#if defined(MS_WINDOWS)
    /* getwindowsversion */
    if (WindowsVersionType.tp_name == 0)
        if (PyStructSequence_InitType2(&WindowsVersionType,
                                       &windows_version_desc) < 0)
            return NULL;
    /* prevent user from creating new instances */
    WindowsVersionType.tp_init = NULL;
    WindowsVersionType.tp_new = NULL;
    res = PyDict_DelItemString(WindowsVersionType.tp_dict, "__new__");
    if (res < 0 && PyErr_ExceptionMatches(PyExc_KeyError))
        PyErr_Clear();
#endif

    /* float repr style: 0.03 (short) vs 0.029999999999999999 (legacy) */
#ifndef PY_NO_SHORT_FLOAT_REPR
    SET_SYS_FROM_STRING("float_repr_style",
                        PyUnicode_FromString("short"));
#else
    SET_SYS_FROM_STRING("float_repr_style",
                        PyUnicode_FromString("legacy"));
#endif

#ifdef WITH_THREAD
    SET_SYS_FROM_STRING("thread_info", PyThread_GetInfo());
#endif

#undef SET_SYS_FROM_STRING
#undef SET_SYS_FROM_STRING_BORROW
    if (PyErr_Occurred())
        return NULL;
    return m;
}

static PyObject *
makepathobject(const wchar_t *path, wchar_t delim)
{
    int i, n;
    const wchar_t *p;
    PyObject *v, *w;

    n = 1;
    p = path;
    while ((p = wcschr(p, delim)) != NULL) {
        n++;
        p++;
    }
    v = PyList_New(n);
    if (v == NULL)
        return NULL;
    for (i = 0; ; i++) {
        p = wcschr(path, delim);
        if (p == NULL)
            p = path + wcslen(path); /* End of string */
        w = PyUnicode_FromWideChar(path, (Py_ssize_t)(p - path));
        if (w == NULL) {
            Py_DECREF(v);
            return NULL;
        }
        PyList_SetItem(v, i, w);
        if (*p == '\0')
            break;
        path = p+1;
    }
    return v;
}

void
PySys_SetPath(const wchar_t *path)
{
    PyObject *v;
    if ((v = makepathobject(path, DELIM)) == NULL)
        Py_FatalError("can't create sys.path");
    if (_PySys_SetObjectId(&PyId_path, v) != 0)
        Py_FatalError("can't assign sys.path");
    Py_DECREF(v);
}

static PyObject *
makeargvobject(int argc, wchar_t **argv)
{
    PyObject *av;
    if (argc <= 0 || argv == NULL) {
        /* Ensure at least one (empty) argument is seen */
        static wchar_t *empty_argv[1] = {L""};
        argv = empty_argv;
        argc = 1;
    }
    av = PyList_New(argc);
    if (av != NULL) {
        int i;
        for (i = 0; i < argc; i++) {
            PyObject *v = PyUnicode_FromWideChar(argv[i], -1);
            if (v == NULL) {
                Py_DECREF(av);
                av = NULL;
                break;
            }
            PyList_SetItem(av, i, v);
        }
    }
    return av;
}

#define _HAVE_SCRIPT_ARGUMENT(argc, argv) \
  (argc > 0 && argv0 != NULL && \
   wcscmp(argv0, L"-c") != 0 && wcscmp(argv0, L"-m") != 0)

static void
sys_update_path(int argc, wchar_t **argv)
{
    wchar_t *argv0;
    wchar_t *p = NULL;
    Py_ssize_t n = 0;
    PyObject *a;
    PyObject *path;
#ifdef HAVE_READLINK
    wchar_t link[MAXPATHLEN+1];
    wchar_t argv0copy[2*MAXPATHLEN+1];
    int nr = 0;
#endif
#if defined(HAVE_REALPATH)
    wchar_t fullpath[MAXPATHLEN];
#elif defined(MS_WINDOWS) && !defined(MS_WINCE)
    wchar_t fullpath[MAX_PATH];
#endif

    path = _PySys_GetObjectId(&PyId_path);
    if (path == NULL)
        return;

    argv0 = argv[0];

#ifdef HAVE_READLINK
    if (_HAVE_SCRIPT_ARGUMENT(argc, argv))
        nr = _Py_wreadlink(argv0, link, MAXPATHLEN);
    if (nr > 0) {
        /* It's a symlink */
        link[nr] = '\0';
        if (link[0] == SEP)
            argv0 = link; /* Link to absolute path */
        else if (wcschr(link, SEP) == NULL)
            ; /* Link without path */
        else {
            /* Must join(dirname(argv0), link) */
            wchar_t *q = wcsrchr(argv0, SEP);
            if (q == NULL)
                argv0 = link; /* argv0 without path */
            else {
                /* Must make a copy, argv0copy has room for 2 * MAXPATHLEN */
                wcsncpy(argv0copy, argv0, MAXPATHLEN);
                q = wcsrchr(argv0copy, SEP);
                wcsncpy(q+1, link, MAXPATHLEN);
                q[MAXPATHLEN + 1] = L'\0';
                argv0 = argv0copy;
            }
        }
    }
#endif /* HAVE_READLINK */
#if SEP == '\\' /* Special case for MS filename syntax */
    if (_HAVE_SCRIPT_ARGUMENT(argc, argv)) {
        wchar_t *q;
#if defined(MS_WINDOWS) && !defined(MS_WINCE)
        /* This code here replaces the first element in argv with the full
        path that it represents. Under CE, there are no relative paths so
        the argument must be the full path anyway. */
        wchar_t *ptemp;
        if (GetFullPathNameW(argv0,
                           Py_ARRAY_LENGTH(fullpath),
                           fullpath,
                           &ptemp)) {
            argv0 = fullpath;
        }
#endif
        p = wcsrchr(argv0, SEP);
        /* Test for alternate separator */
        q = wcsrchr(p ? p : argv0, '/');
        if (q != NULL)
            p = q;
        if (p != NULL) {
            n = p + 1 - argv0;
            if (n > 1 && p[-1] != ':')
                n--; /* Drop trailing separator */
        }
    }
#else /* All other filename syntaxes */
    if (_HAVE_SCRIPT_ARGUMENT(argc, argv)) {
#if defined(HAVE_REALPATH)
        if (_Py_wrealpath(argv0, fullpath, Py_ARRAY_LENGTH(fullpath))) {
            argv0 = fullpath;
        }
#endif
        p = wcsrchr(argv0, SEP);
    }
    if (p != NULL) {
        n = p + 1 - argv0;
#if SEP == '/' /* Special case for Unix filename syntax */
        if (n > 1)
            n--; /* Drop trailing separator */
#endif /* Unix */
    }
#endif /* All others */
    a = PyUnicode_FromWideChar(argv0, n);
    if (a == NULL)
        Py_FatalError("no mem for sys.path insertion");
    if (PyList_Insert(path, 0, a) < 0)
        Py_FatalError("sys.path.insert(0) failed");
    Py_DECREF(a);
}

void
PySys_SetArgvEx(int argc, wchar_t **argv, int updatepath)
{
    PyObject *av = makeargvobject(argc, argv);
    if (av == NULL)
        Py_FatalError("no mem for sys.argv");
    if (PySys_SetObject("argv", av) != 0)
        Py_FatalError("can't assign sys.argv");
    Py_DECREF(av);
    if (updatepath)
        sys_update_path(argc, argv);
}

void
PySys_SetArgv(int argc, wchar_t **argv)
{
    PySys_SetArgvEx(argc, argv, Py_IsolatedFlag == 0);
}

/* Reimplementation of PyFile_WriteString() no calling indirectly
   PyErr_CheckSignals(): avoid the call to PyObject_Str(). */

static int
sys_pyfile_write_unicode(PyObject *unicode, PyObject *file)
{
    PyObject *writer = NULL, *args = NULL, *result = NULL;
    int err;

    if (file == NULL)
        return -1;

    writer = _PyObject_GetAttrId(file, &PyId_write);
    if (writer == NULL)
        goto error;

    args = PyTuple_Pack(1, unicode);
    if (args == NULL)
        goto error;

    result = PyEval_CallObject(writer, args);
    if (result == NULL) {
        goto error;
    } else {
        err = 0;
        goto finally;
    }

error:
    err = -1;
finally:
    Py_XDECREF(writer);
    Py_XDECREF(args);
    Py_XDECREF(result);
    return err;
}

static int
sys_pyfile_write(const char *text, PyObject *file)
{
    PyObject *unicode = NULL;
    int err;

    if (file == NULL)
        return -1;

    unicode = PyUnicode_FromString(text);
    if (unicode == NULL)
        return -1;

    err = sys_pyfile_write_unicode(unicode, file);
    Py_DECREF(unicode);
    return err;
}

/* APIs to write to sys.stdout or sys.stderr using a printf-like interface.
   Adapted from code submitted by Just van Rossum.

   PySys_WriteStdout(format, ...)
   PySys_WriteStderr(format, ...)

      The first function writes to sys.stdout; the second to sys.stderr.  When
      there is a problem, they write to the real (C level) stdout or stderr;
      no exceptions are raised.

      PyErr_CheckSignals() is not called to avoid the execution of the Python
      signal handlers: they may raise a new exception whereas sys_write()
      ignores all exceptions.

      Both take a printf-style format string as their first argument followed
      by a variable length argument list determined by the format string.

      *** WARNING ***

      The format should limit the total size of the formatted output string to
      1000 bytes.  In particular, this means that no unrestricted "%s" formats
      should occur; these should be limited using "%.<N>s where <N> is a
      decimal number calculated so that <N> plus the maximum size of other
      formatted text does not exceed 1000 bytes.  Also watch out for "%f",
      which can print hundreds of digits for very large numbers.

 */

static void
sys_write(_Py_Identifier *key, FILE *fp, const char *format, va_list va)
{
    PyObject *file;
    PyObject *error_type, *error_value, *error_traceback;
    char buffer[1001];
    int written;

    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    file = _PySys_GetObjectId(key);
    written = PyOS_vsnprintf(buffer, sizeof(buffer), format, va);
    if (sys_pyfile_write(buffer, file) != 0) {
        PyErr_Clear();
        fputs(buffer, fp);
    }
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        const char *truncated = "... truncated";
        if (sys_pyfile_write(truncated, file) != 0)
            fputs(truncated, fp);
    }
    PyErr_Restore(error_type, error_value, error_traceback);
}

void
PySys_WriteStdout(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    sys_write(&PyId_stdout, stdout, format, va);
    va_end(va);
}

void
PySys_WriteStderr(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    sys_write(&PyId_stderr, stderr, format, va);
    va_end(va);
}

static void
sys_format(_Py_Identifier *key, FILE *fp, const char *format, va_list va)
{
    PyObject *file, *message;
    PyObject *error_type, *error_value, *error_traceback;
    char *utf8;

    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    file = _PySys_GetObjectId(key);
    message = PyUnicode_FromFormatV(format, va);
    if (message != NULL) {
        if (sys_pyfile_write_unicode(message, file) != 0) {
            PyErr_Clear();
            utf8 = _PyUnicode_AsString(message);
            if (utf8 != NULL)
                fputs(utf8, fp);
        }
        Py_DECREF(message);
    }
    PyErr_Restore(error_type, error_value, error_traceback);
}

void
PySys_FormatStdout(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    sys_format(&PyId_stdout, stdout, format, va);
    va_end(va);
}

void
PySys_FormatStderr(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    sys_format(&PyId_stderr, stderr, format, va);
    va_end(va);
}
