/* File object implementation (what's left of it -- see io.py) */

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#ifdef HAVE_GETC_UNLOCKED
#define GETC(f) getc_unlocked(f)
#define FLOCKFILE(f) flockfile(f)
#define FUNLOCKFILE(f) funlockfile(f)
#else
#define GETC(f) getc(f)
#define FLOCKFILE(f)
#define FUNLOCKFILE(f)
#endif

/* Newline flags */
#define NEWLINE_UNKNOWN 0       /* No newline seen, yet */
#define NEWLINE_CR 1            /* \r newline seen */
#define NEWLINE_LF 2            /* \n newline seen */
#define NEWLINE_CRLF 4          /* \r\n newline seen */

#ifdef __cplusplus
extern "C" {
#endif

/* External C interface */

PyObject *
PyFile_FromFd(int fd, const char *name, const char *mode, int buffering, const char *encoding,
              const char *errors, const char *newline, int closefd)
{
    PyObject *io, *stream;
    _Py_IDENTIFIER(open);

    io = PyImport_ImportModule("io");
    if (io == NULL)
        return NULL;
    stream = _PyObject_CallMethodId(io, &PyId_open, "isisssi", fd, mode,
                                 buffering, encoding, errors,
                                 newline, closefd);
    Py_DECREF(io);
    if (stream == NULL)
        return NULL;
    /* ignore name attribute because the name attribute of _BufferedIOMixin
       and TextIOWrapper is read only */
    return stream;
}

PyObject *
PyFile_GetLine(PyObject *f, int n)
{
    PyObject *result;

    if (f == NULL) {
        PyErr_BadInternalCall();
        return NULL;
    }

    {
        PyObject *reader;
        PyObject *args;
        _Py_IDENTIFIER(readline);

        reader = _PyObject_GetAttrId(f, &PyId_readline);
        if (reader == NULL)
            return NULL;
        if (n <= 0)
            args = PyTuple_New(0);
        else
            args = Py_BuildValue("(i)", n);
        if (args == NULL) {
            Py_DECREF(reader);
            return NULL;
        }
        result = PyEval_CallObject(reader, args);
        Py_DECREF(reader);
        Py_DECREF(args);
        if (result != NULL && !PyBytes_Check(result) &&
            !PyUnicode_Check(result)) {
            Py_DECREF(result);
            result = NULL;
            PyErr_SetString(PyExc_TypeError,
                       "object.readline() returned non-string");
        }
    }

    if (n < 0 && result != NULL && PyBytes_Check(result)) {
        char *s = PyBytes_AS_STRING(result);
        Py_ssize_t len = PyBytes_GET_SIZE(result);
        if (len == 0) {
            Py_DECREF(result);
            result = NULL;
            PyErr_SetString(PyExc_EOFError,
                            "EOF when reading a line");
        }
        else if (s[len-1] == '\n') {
            if (result->ob_refcnt == 1)
                _PyBytes_Resize(&result, len-1);
            else {
                PyObject *v;
                v = PyBytes_FromStringAndSize(s, len-1);
                Py_DECREF(result);
                result = v;
            }
        }
    }
    if (n < 0 && result != NULL && PyUnicode_Check(result)) {
        Py_ssize_t len = PyUnicode_GET_LENGTH(result);
        if (len == 0) {
            Py_DECREF(result);
            result = NULL;
            PyErr_SetString(PyExc_EOFError,
                            "EOF when reading a line");
        }
        else if (PyUnicode_READ_CHAR(result, len-1) == '\n') {
            PyObject *v;
            v = PyUnicode_Substring(result, 0, len-1);
            Py_DECREF(result);
            result = v;
        }
    }
    return result;
}

/* Interfaces to write objects/strings to file-like objects */

int
PyFile_WriteObject(PyObject *v, PyObject *f, int flags)
{
    PyObject *writer, *value, *args, *result;
    _Py_IDENTIFIER(write);

    if (f == NULL) {
        PyErr_SetString(PyExc_TypeError, "writeobject with NULL file");
        return -1;
    }
    writer = _PyObject_GetAttrId(f, &PyId_write);
    if (writer == NULL)
        return -1;
    if (flags & Py_PRINT_RAW) {
        value = PyObject_Str(v);
    }
    else
        value = PyObject_Repr(v);
    if (value == NULL) {
        Py_DECREF(writer);
        return -1;
    }
    args = PyTuple_Pack(1, value);
    if (args == NULL) {
        Py_DECREF(value);
        Py_DECREF(writer);
        return -1;
    }
    result = PyEval_CallObject(writer, args);
    Py_DECREF(args);
    Py_DECREF(value);
    Py_DECREF(writer);
    if (result == NULL)
        return -1;
    Py_DECREF(result);
    return 0;
}

int
PyFile_WriteString(const char *s, PyObject *f)
{
    if (f == NULL) {
        /* Should be caused by a pre-existing error */
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_SystemError,
                            "null file for PyFile_WriteString");
        return -1;
    }
    else if (!PyErr_Occurred()) {
        PyObject *v = PyUnicode_FromString(s);
        int err;
        if (v == NULL)
            return -1;
        err = PyFile_WriteObject(v, f, Py_PRINT_RAW);
        Py_DECREF(v);
        return err;
    }
    else
        return -1;
}

/* Try to get a file-descriptor from a Python object.  If the object
   is an integer, its value is returned.  If not, the
   object's fileno() method is called if it exists; the method must return
   an integer, which is returned as the file descriptor value.
   -1 is returned on failure.
*/

int
PyObject_AsFileDescriptor(PyObject *o)
{
    int fd;
    PyObject *meth;
    _Py_IDENTIFIER(fileno);

    if (PyLong_Check(o)) {
        fd = _PyLong_AsInt(o);
    }
    else if ((meth = _PyObject_GetAttrId(o, &PyId_fileno)) != NULL)
    {
        PyObject *fno = PyEval_CallObject(meth, NULL);
        Py_DECREF(meth);
        if (fno == NULL)
            return -1;

        if (PyLong_Check(fno)) {
            fd = _PyLong_AsInt(fno);
            Py_DECREF(fno);
        }
        else {
            PyErr_SetString(PyExc_TypeError,
                            "fileno() returned a non-integer");
            Py_DECREF(fno);
            return -1;
        }
    }
    else {
        PyErr_SetString(PyExc_TypeError,
                        "argument must be an int, or have a fileno() method.");
        return -1;
    }

    if (fd == -1 && PyErr_Occurred())
        return -1;
    if (fd < 0) {
        PyErr_Format(PyExc_ValueError,
                     "file descriptor cannot be a negative integer (%i)",
                     fd);
        return -1;
    }
    return fd;
}

/*
** Py_UniversalNewlineFgets is an fgets variation that understands
** all of \r, \n and \r\n conventions.
** The stream should be opened in binary mode.
** If fobj is NULL the routine always does newline conversion, and
** it may peek one char ahead to gobble the second char in \r\n.
** If fobj is non-NULL it must be a PyFileObject. In this case there
** is no readahead but in stead a flag is used to skip a following
** \n on the next read. Also, if the file is open in binary mode
** the whole conversion is skipped. Finally, the routine keeps track of
** the different types of newlines seen.
** Note that we need no error handling: fgets() treats error and eof
** identically.
*/
char *
Py_UniversalNewlineFgets(char *buf, int n, FILE *stream, PyObject *fobj)
{
    char *p = buf;
    int c;
    int newlinetypes = 0;
    int skipnextlf = 0;

    if (fobj) {
        errno = ENXIO;          /* What can you do... */
        return NULL;
    }
    FLOCKFILE(stream);
    c = 'x'; /* Shut up gcc warning */
    while (--n > 0 && (c = GETC(stream)) != EOF ) {
        if (skipnextlf ) {
            skipnextlf = 0;
            if (c == '\n') {
                /* Seeing a \n here with skipnextlf true
                ** means we saw a \r before.
                */
                newlinetypes |= NEWLINE_CRLF;
                c = GETC(stream);
                if (c == EOF) break;
            } else {
                /*
                ** Note that c == EOF also brings us here,
                ** so we're okay if the last char in the file
                ** is a CR.
                */
                newlinetypes |= NEWLINE_CR;
            }
        }
        if (c == '\r') {
            /* A \r is translated into a \n, and we skip
            ** an adjacent \n, if any. We don't set the
            ** newlinetypes flag until we've seen the next char.
            */
            skipnextlf = 1;
            c = '\n';
        } else if ( c == '\n') {
            newlinetypes |= NEWLINE_LF;
        }
        *p++ = c;
        if (c == '\n') break;
    }
    /* if ( c == EOF && skipnextlf )
        newlinetypes |= NEWLINE_CR; */
    FUNLOCKFILE(stream);
    *p = '\0';
    if ( skipnextlf ) {
        /* If we have no file object we cannot save the
        ** skipnextlf flag. We have to readahead, which
        ** will cause a pause if we're reading from an
        ** interactive stream, but that is very unlikely
        ** unless we're doing something silly like
        ** exec(open("/dev/tty").read()).
        */
        c = GETC(stream);
        if ( c != '\n' )
            ungetc(c, stream);
    }
    if (p == buf)
        return NULL;
    return buf;
}

/* **************************** std printer ****************************
 * The stdprinter is used during the boot strapping phase as a preliminary
 * file like object for sys.stderr.
 */

typedef struct {
    PyObject_HEAD
    int fd;
} PyStdPrinter_Object;

static PyObject *
stdprinter_new(PyTypeObject *type, PyObject *args, PyObject *kews)
{
    PyStdPrinter_Object *self;

    assert(type != NULL && type->tp_alloc != NULL);

    self = (PyStdPrinter_Object *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->fd = -1;
    }

    return (PyObject *) self;
}

static int
stdprinter_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyErr_SetString(PyExc_TypeError,
                    "cannot create 'stderrprinter' instances");
    return -1;
}

PyObject *
PyFile_NewStdPrinter(int fd)
{
    PyStdPrinter_Object *self;

    if (fd != fileno(stdout) && fd != fileno(stderr)) {
        /* not enough infrastructure for PyErr_BadInternalCall() */
        return NULL;
    }

    self = PyObject_New(PyStdPrinter_Object,
                        &PyStdPrinter_Type);
    if (self != NULL) {
        self->fd = fd;
    }
    return (PyObject*)self;
}

static PyObject *
stdprinter_write(PyStdPrinter_Object *self, PyObject *args)
{
    char *c;
    Py_ssize_t n;

    if (self->fd < 0) {
        /* fd might be invalid on Windows
         * I can't raise an exception here. It may lead to an
         * unlimited recursion in the case stderr is invalid.
         */
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, "s", &c)) {
        return NULL;
    }
    n = strlen(c);

    Py_BEGIN_ALLOW_THREADS
    errno = 0;
#ifdef MS_WINDOWS
    if (n > INT_MAX)
        n = INT_MAX;
    n = write(self->fd, c, (int)n);
#else
    n = write(self->fd, c, n);
#endif
    Py_END_ALLOW_THREADS

    if (n < 0) {
        if (errno == EAGAIN)
            Py_RETURN_NONE;
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }

    return PyLong_FromSsize_t(n);
}

static PyObject *
stdprinter_fileno(PyStdPrinter_Object *self)
{
    return PyLong_FromLong((long) self->fd);
}

static PyObject *
stdprinter_repr(PyStdPrinter_Object *self)
{
    return PyUnicode_FromFormat("<stdprinter(fd=%d) object at 0x%x>",
                                self->fd, self);
}

static PyObject *
stdprinter_noop(PyStdPrinter_Object *self)
{
    Py_RETURN_NONE;
}

static PyObject *
stdprinter_isatty(PyStdPrinter_Object *self)
{
    long res;
    if (self->fd < 0) {
        Py_RETURN_FALSE;
    }

    Py_BEGIN_ALLOW_THREADS
    res = isatty(self->fd);
    Py_END_ALLOW_THREADS

    return PyBool_FromLong(res);
}

static PyMethodDef stdprinter_methods[] = {
    {"close",           (PyCFunction)stdprinter_noop, METH_NOARGS, ""},
    {"flush",           (PyCFunction)stdprinter_noop, METH_NOARGS, ""},
    {"fileno",          (PyCFunction)stdprinter_fileno, METH_NOARGS, ""},
    {"isatty",          (PyCFunction)stdprinter_isatty, METH_NOARGS, ""},
    {"write",           (PyCFunction)stdprinter_write, METH_VARARGS, ""},
    {NULL,              NULL}  /*sentinel */
};

static PyObject *
get_closed(PyStdPrinter_Object *self, void *closure)
{
    Py_INCREF(Py_False);
    return Py_False;
}

static PyObject *
get_mode(PyStdPrinter_Object *self, void *closure)
{
    return PyUnicode_FromString("w");
}

static PyObject *
get_encoding(PyStdPrinter_Object *self, void *closure)
{
    Py_RETURN_NONE;
}

static PyGetSetDef stdprinter_getsetlist[] = {
    {"closed", (getter)get_closed, NULL, "True if the file is closed"},
    {"encoding", (getter)get_encoding, NULL, "Encoding of the file"},
    {"mode", (getter)get_mode, NULL, "String giving the file mode"},
    {0},
};

PyTypeObject PyStdPrinter_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "stderrprinter",                            /* tp_name */
    sizeof(PyStdPrinter_Object),                /* tp_basicsize */
    0,                                          /* tp_itemsize */
    /* methods */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc)stdprinter_repr,                  /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    stdprinter_methods,                         /* tp_methods */
    0,                                          /* tp_members */
    stdprinter_getsetlist,                      /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    stdprinter_init,                            /* tp_init */
    PyType_GenericAlloc,                        /* tp_alloc */
    stdprinter_new,                             /* tp_new */
    PyObject_Del,                               /* tp_free */
};


#ifdef __cplusplus
}
#endif
