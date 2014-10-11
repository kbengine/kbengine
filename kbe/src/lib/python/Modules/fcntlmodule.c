
/* fcntl module */

#define PY_SSIZE_T_CLEAN

#include "Python.h"

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#include <sys/ioctl.h>
#include <fcntl.h>
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

static int
conv_descriptor(PyObject *object, int *target)
{
    int fd = PyObject_AsFileDescriptor(object);

    if (fd < 0)
    return 0;
    *target = fd;
    return 1;
}


/* fcntl(fd, op, [arg]) */

static PyObject *
fcntl_fcntl(PyObject *self, PyObject *args)
{
    int fd;
    int code;
    long arg;
    int ret;
    char *str;
    Py_ssize_t len;
    char buf[1024];

    if (PyArg_ParseTuple(args, "O&is#:fcntl",
                         conv_descriptor, &fd, &code, &str, &len)) {
        if (len > sizeof buf) {
            PyErr_SetString(PyExc_ValueError,
                            "fcntl string arg too long");
            return NULL;
        }
        memcpy(buf, str, len);
        Py_BEGIN_ALLOW_THREADS
        ret = fcntl(fd, code, buf);
        Py_END_ALLOW_THREADS
        if (ret < 0) {
            PyErr_SetFromErrno(PyExc_IOError);
            return NULL;
        }
        return PyBytes_FromStringAndSize(buf, len);
    }

    PyErr_Clear();
    arg = 0;
    if (!PyArg_ParseTuple(args,
         "O&i|l;fcntl requires a file or file descriptor,"
         " an integer and optionally a third integer or a string",
                          conv_descriptor, &fd, &code, &arg)) {
      return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    ret = fcntl(fd, code, arg);
    Py_END_ALLOW_THREADS
    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    return PyLong_FromLong((long)ret);
}

PyDoc_STRVAR(fcntl_doc,
"fcntl(fd, op, [arg])\n\
\n\
Perform the operation op on file descriptor fd.  The values used\n\
for op are operating system dependent, and are available\n\
as constants in the fcntl module, using the same names as used in\n\
the relevant C header files.  The argument arg is optional, and\n\
defaults to 0; it may be an int or a string.  If arg is given as a string,\n\
the return value of fcntl is a string of that length, containing the\n\
resulting value put in the arg buffer by the operating system.  The length\n\
of the arg string is not allowed to exceed 1024 bytes.  If the arg given\n\
is an integer or if none is specified, the result value is an integer\n\
corresponding to the return value of the fcntl call in the C code.");


/* ioctl(fd, op, [arg]) */

static PyObject *
fcntl_ioctl(PyObject *self, PyObject *args)
{
#define IOCTL_BUFSZ 1024
    int fd;
    /* In PyArg_ParseTuple below, we use the unsigned non-checked 'I'
       format for the 'code' parameter because Python turns 0x8000000
       into either a large positive number (PyLong or PyInt on 64-bit
       platforms) or a negative number on others (32-bit PyInt)
       whereas the system expects it to be a 32bit bit field value
       regardless of it being passed as an int or unsigned long on
       various platforms.  See the termios.TIOCSWINSZ constant across
       platforms for an example of this.

       If any of the 64bit platforms ever decide to use more than 32bits
       in their unsigned long ioctl codes this will break and need
       special casing based on the platform being built on.
     */
    unsigned int code;
    int arg;
    int ret;
    Py_buffer pstr;
    char *str;
    Py_ssize_t len;
    int mutate_arg = 1;
    char buf[IOCTL_BUFSZ+1];  /* argument plus NUL byte */

    if (PyArg_ParseTuple(args, "O&Iw*|i:ioctl",
                         conv_descriptor, &fd, &code,
                         &pstr, &mutate_arg)) {
        char *arg;
        str = pstr.buf;
        len = pstr.len;

        if (mutate_arg) {
            if (len <= IOCTL_BUFSZ) {
                memcpy(buf, str, len);
                buf[len] = '\0';
                arg = buf;
            }
            else {
                arg = str;
            }
        }
        else {
            if (len > IOCTL_BUFSZ) {
                PyBuffer_Release(&pstr);
                PyErr_SetString(PyExc_ValueError,
                    "ioctl string arg too long");
                return NULL;
            }
            else {
                memcpy(buf, str, len);
                buf[len] = '\0';
                arg = buf;
            }
        }
        if (buf == arg) {
            Py_BEGIN_ALLOW_THREADS /* think array.resize() */
            ret = ioctl(fd, code, arg);
            Py_END_ALLOW_THREADS
        }
        else {
            ret = ioctl(fd, code, arg);
        }
        if (mutate_arg && (len <= IOCTL_BUFSZ)) {
            memcpy(str, buf, len);
        }
        PyBuffer_Release(&pstr); /* No further access to str below this point */
        if (ret < 0) {
            PyErr_SetFromErrno(PyExc_IOError);
            return NULL;
        }
        if (mutate_arg) {
            return PyLong_FromLong(ret);
        }
        else {
            return PyBytes_FromStringAndSize(buf, len);
        }
    }

    PyErr_Clear();
    if (PyArg_ParseTuple(args, "O&Is*:ioctl",
                         conv_descriptor, &fd, &code, &pstr)) {
        str = pstr.buf;
        len = pstr.len;
        if (len > IOCTL_BUFSZ) {
            PyBuffer_Release(&pstr);
            PyErr_SetString(PyExc_ValueError,
                            "ioctl string arg too long");
            return NULL;
        }
        memcpy(buf, str, len);
        buf[len] = '\0';
        Py_BEGIN_ALLOW_THREADS
        ret = ioctl(fd, code, buf);
        Py_END_ALLOW_THREADS
        if (ret < 0) {
            PyBuffer_Release(&pstr);
            PyErr_SetFromErrno(PyExc_IOError);
            return NULL;
        }
        PyBuffer_Release(&pstr);
        return PyBytes_FromStringAndSize(buf, len);
    }

    PyErr_Clear();
    arg = 0;
    if (!PyArg_ParseTuple(args,
         "O&I|i;ioctl requires a file or file descriptor,"
         " an integer and optionally an integer or buffer argument",
                          conv_descriptor, &fd, &code, &arg)) {
      return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    ret = ioctl(fd, code, arg);
    Py_END_ALLOW_THREADS
    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    return PyLong_FromLong((long)ret);
#undef IOCTL_BUFSZ
}

PyDoc_STRVAR(ioctl_doc,
"ioctl(fd, op[, arg[, mutate_flag]])\n\
\n\
Perform the operation op on file descriptor fd.  The values used for op\n\
are operating system dependent, and are available as constants in the\n\
fcntl or termios library modules, using the same names as used in the\n\
relevant C header files.\n\
\n\
The argument arg is optional, and defaults to 0; it may be an int or a\n\
buffer containing character data (most likely a string or an array). \n\
\n\
If the argument is a mutable buffer (such as an array) and if the\n\
mutate_flag argument (which is only allowed in this case) is true then the\n\
buffer is (in effect) passed to the operating system and changes made by\n\
the OS will be reflected in the contents of the buffer after the call has\n\
returned.  The return value is the integer returned by the ioctl system\n\
call.\n\
\n\
If the argument is a mutable buffer and the mutable_flag argument is not\n\
passed or is false, the behavior is as if a string had been passed.  This\n\
behavior will change in future releases of Python.\n\
\n\
If the argument is an immutable buffer (most likely a string) then a copy\n\
of the buffer is passed to the operating system and the return value is a\n\
string of the same length containing whatever the operating system put in\n\
the buffer.  The length of the arg buffer in this case is not allowed to\n\
exceed 1024 bytes.\n\
\n\
If the arg given is an integer or if none is specified, the result value is\n\
an integer corresponding to the return value of the ioctl call in the C\n\
code.");


/* flock(fd, operation) */

static PyObject *
fcntl_flock(PyObject *self, PyObject *args)
{
    int fd;
    int code;
    int ret;

    if (!PyArg_ParseTuple(args, "O&i:flock",
                          conv_descriptor, &fd, &code))
        return NULL;

#ifdef HAVE_FLOCK
    Py_BEGIN_ALLOW_THREADS
    ret = flock(fd, code);
    Py_END_ALLOW_THREADS
#else

#ifndef LOCK_SH
#define LOCK_SH         1       /* shared lock */
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_NB         4       /* don't block when locking */
#define LOCK_UN         8       /* unlock */
#endif
    {
        struct flock l;
        if (code == LOCK_UN)
            l.l_type = F_UNLCK;
        else if (code & LOCK_SH)
            l.l_type = F_RDLCK;
        else if (code & LOCK_EX)
            l.l_type = F_WRLCK;
        else {
            PyErr_SetString(PyExc_ValueError,
                            "unrecognized flock argument");
            return NULL;
        }
        l.l_whence = l.l_start = l.l_len = 0;
        Py_BEGIN_ALLOW_THREADS
        ret = fcntl(fd, (code & LOCK_NB) ? F_SETLK : F_SETLKW, &l);
        Py_END_ALLOW_THREADS
    }
#endif /* HAVE_FLOCK */
    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(flock_doc,
"flock(fd, operation)\n\
\n\
Perform the lock operation op on file descriptor fd.  See the Unix \n\
manual page for flock(2) for details.  (On some systems, this function is\n\
emulated using fcntl().)");


/* lockf(fd, operation) */
static PyObject *
fcntl_lockf(PyObject *self, PyObject *args)
{
    int fd, code, ret, whence = 0;
    PyObject *lenobj = NULL, *startobj = NULL;

    if (!PyArg_ParseTuple(args, "O&i|OOi:lockf",
                          conv_descriptor, &fd, &code,
                          &lenobj, &startobj, &whence))
        return NULL;

#ifndef LOCK_SH
#define LOCK_SH         1       /* shared lock */
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_NB         4       /* don't block when locking */
#define LOCK_UN         8       /* unlock */
#endif  /* LOCK_SH */
    {
        struct flock l;
        if (code == LOCK_UN)
            l.l_type = F_UNLCK;
        else if (code & LOCK_SH)
            l.l_type = F_RDLCK;
        else if (code & LOCK_EX)
            l.l_type = F_WRLCK;
        else {
            PyErr_SetString(PyExc_ValueError,
                            "unrecognized lockf argument");
            return NULL;
        }
        l.l_start = l.l_len = 0;
        if (startobj != NULL) {
#if !defined(HAVE_LARGEFILE_SUPPORT)
            l.l_start = PyLong_AsLong(startobj);
#else
            l.l_start = PyLong_Check(startobj) ?
                            PyLong_AsLongLong(startobj) :
                    PyLong_AsLong(startobj);
#endif
            if (PyErr_Occurred())
                return NULL;
        }
        if (lenobj != NULL) {
#if !defined(HAVE_LARGEFILE_SUPPORT)
            l.l_len = PyLong_AsLong(lenobj);
#else
            l.l_len = PyLong_Check(lenobj) ?
                            PyLong_AsLongLong(lenobj) :
                    PyLong_AsLong(lenobj);
#endif
            if (PyErr_Occurred())
                return NULL;
        }
        l.l_whence = whence;
        Py_BEGIN_ALLOW_THREADS
        ret = fcntl(fd, (code & LOCK_NB) ? F_SETLK : F_SETLKW, &l);
        Py_END_ALLOW_THREADS
    }
    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

PyDoc_STRVAR(lockf_doc,
"lockf (fd, operation, length=0, start=0, whence=0)\n\
\n\
This is essentially a wrapper around the fcntl() locking calls.  fd is the\n\
file descriptor of the file to lock or unlock, and operation is one of the\n\
following values:\n\
\n\
    LOCK_UN - unlock\n\
    LOCK_SH - acquire a shared lock\n\
    LOCK_EX - acquire an exclusive lock\n\
\n\
When operation is LOCK_SH or LOCK_EX, it can also be bitwise ORed with\n\
LOCK_NB to avoid blocking on lock acquisition.  If LOCK_NB is used and the\n\
lock cannot be acquired, an IOError will be raised and the exception will\n\
have an errno attribute set to EACCES or EAGAIN (depending on the operating\n\
system -- for portability, check for either value).\n\
\n\
length is the number of bytes to lock, with the default meaning to lock to\n\
EOF.  start is the byte offset, relative to whence, to that the lock\n\
starts.  whence is as with fileobj.seek(), specifically:\n\
\n\
    0 - relative to the start of the file (SEEK_SET)\n\
    1 - relative to the current buffer position (SEEK_CUR)\n\
    2 - relative to the end of the file (SEEK_END)");

/* List of functions */

static PyMethodDef fcntl_methods[] = {
    {"fcntl",           fcntl_fcntl, METH_VARARGS, fcntl_doc},
    {"ioctl",           fcntl_ioctl, METH_VARARGS, ioctl_doc},
    {"flock",           fcntl_flock, METH_VARARGS, flock_doc},
    {"lockf",       fcntl_lockf, METH_VARARGS, lockf_doc},
    {NULL,              NULL}           /* sentinel */
};


PyDoc_STRVAR(module_doc,
"This module performs file control and I/O control on file \n\
descriptors.  It is an interface to the fcntl() and ioctl() Unix\n\
routines.  File descriptors can be obtained with the fileno() method of\n\
a file or socket object.");

/* Module initialisation */


static int
all_ins(PyObject* m)
{
    if (PyModule_AddIntMacro(m, LOCK_SH)) return -1;
    if (PyModule_AddIntMacro(m, LOCK_EX)) return -1;
    if (PyModule_AddIntMacro(m, LOCK_NB)) return -1;
    if (PyModule_AddIntMacro(m, LOCK_UN)) return -1;
/* GNU extensions, as of glibc 2.2.4 */
#ifdef LOCK_MAND
    if (PyModule_AddIntMacro(m, LOCK_MAND)) return -1;
#endif
#ifdef LOCK_READ
    if (PyModule_AddIntMacro(m, LOCK_READ)) return -1;
#endif
#ifdef LOCK_WRITE
    if (PyModule_AddIntMacro(m, LOCK_WRITE)) return -1;
#endif
#ifdef LOCK_RW
    if (PyModule_AddIntMacro(m, LOCK_RW)) return -1;
#endif

#ifdef F_DUPFD
    if (PyModule_AddIntMacro(m, F_DUPFD)) return -1;
#endif
#ifdef F_DUPFD_CLOEXEC
    if (PyModule_AddIntMacro(m, F_DUPFD_CLOEXEC)) return -1;
#endif
#ifdef F_GETFD
    if (PyModule_AddIntMacro(m, F_GETFD)) return -1;
#endif
#ifdef F_SETFD
    if (PyModule_AddIntMacro(m, F_SETFD)) return -1;
#endif
#ifdef F_GETFL
    if (PyModule_AddIntMacro(m, F_GETFL)) return -1;
#endif
#ifdef F_SETFL
    if (PyModule_AddIntMacro(m, F_SETFL)) return -1;
#endif
#ifdef F_GETLK
    if (PyModule_AddIntMacro(m, F_GETLK)) return -1;
#endif
#ifdef F_SETLK
    if (PyModule_AddIntMacro(m, F_SETLK)) return -1;
#endif
#ifdef F_SETLKW
    if (PyModule_AddIntMacro(m, F_SETLKW)) return -1;
#endif
#ifdef F_GETOWN
    if (PyModule_AddIntMacro(m, F_GETOWN)) return -1;
#endif
#ifdef F_SETOWN
    if (PyModule_AddIntMacro(m, F_SETOWN)) return -1;
#endif
#ifdef F_GETSIG
    if (PyModule_AddIntMacro(m, F_GETSIG)) return -1;
#endif
#ifdef F_SETSIG
    if (PyModule_AddIntMacro(m, F_SETSIG)) return -1;
#endif
#ifdef F_RDLCK
    if (PyModule_AddIntMacro(m, F_RDLCK)) return -1;
#endif
#ifdef F_WRLCK
    if (PyModule_AddIntMacro(m, F_WRLCK)) return -1;
#endif
#ifdef F_UNLCK
    if (PyModule_AddIntMacro(m, F_UNLCK)) return -1;
#endif
/* LFS constants */
#ifdef F_GETLK64
    if (PyModule_AddIntMacro(m, F_GETLK64)) return -1;
#endif
#ifdef F_SETLK64
    if (PyModule_AddIntMacro(m, F_SETLK64)) return -1;
#endif
#ifdef F_SETLKW64
    if (PyModule_AddIntMacro(m, F_SETLKW64)) return -1;
#endif
/* GNU extensions, as of glibc 2.2.4. */
#ifdef FASYNC
    if (PyModule_AddIntMacro(m, FASYNC)) return -1;
#endif
#ifdef F_SETLEASE
    if (PyModule_AddIntMacro(m, F_SETLEASE)) return -1;
#endif
#ifdef F_GETLEASE
    if (PyModule_AddIntMacro(m, F_GETLEASE)) return -1;
#endif
#ifdef F_NOTIFY
    if (PyModule_AddIntMacro(m, F_NOTIFY)) return -1;
#endif
/* Old BSD flock(). */
#ifdef F_EXLCK
    if (PyModule_AddIntMacro(m, F_EXLCK)) return -1;
#endif
#ifdef F_SHLCK
    if (PyModule_AddIntMacro(m, F_SHLCK)) return -1;
#endif

/* OS X specifics */
#ifdef F_FULLFSYNC
    if (PyModule_AddIntMacro(m, F_FULLFSYNC)) return -1;
#endif
#ifdef F_NOCACHE
    if (PyModule_AddIntMacro(m, F_NOCACHE)) return -1;
#endif

/* For F_{GET|SET}FL */
#ifdef FD_CLOEXEC
    if (PyModule_AddIntMacro(m, FD_CLOEXEC)) return -1;
#endif

/* For F_NOTIFY */
#ifdef DN_ACCESS
    if (PyModule_AddIntMacro(m, DN_ACCESS)) return -1;
#endif
#ifdef DN_MODIFY
    if (PyModule_AddIntMacro(m, DN_MODIFY)) return -1;
#endif
#ifdef DN_CREATE
    if (PyModule_AddIntMacro(m, DN_CREATE)) return -1;
#endif
#ifdef DN_DELETE
    if (PyModule_AddIntMacro(m, DN_DELETE)) return -1;
#endif
#ifdef DN_RENAME
    if (PyModule_AddIntMacro(m, DN_RENAME)) return -1;
#endif
#ifdef DN_ATTRIB
    if (PyModule_AddIntMacro(m, DN_ATTRIB)) return -1;
#endif
#ifdef DN_MULTISHOT
    if (PyModule_AddIntMacro(m, DN_MULTISHOT)) return -1;
#endif

#ifdef HAVE_STROPTS_H
    /* Unix 98 guarantees that these are in stropts.h. */
    if (PyModule_AddIntMacro(m, I_PUSH)) return -1;
    if (PyModule_AddIntMacro(m, I_POP)) return -1;
    if (PyModule_AddIntMacro(m, I_LOOK)) return -1;
    if (PyModule_AddIntMacro(m, I_FLUSH)) return -1;
    if (PyModule_AddIntMacro(m, I_FLUSHBAND)) return -1;
    if (PyModule_AddIntMacro(m, I_SETSIG)) return -1;
    if (PyModule_AddIntMacro(m, I_GETSIG)) return -1;
    if (PyModule_AddIntMacro(m, I_FIND)) return -1;
    if (PyModule_AddIntMacro(m, I_PEEK)) return -1;
    if (PyModule_AddIntMacro(m, I_SRDOPT)) return -1;
    if (PyModule_AddIntMacro(m, I_GRDOPT)) return -1;
    if (PyModule_AddIntMacro(m, I_NREAD)) return -1;
    if (PyModule_AddIntMacro(m, I_FDINSERT)) return -1;
    if (PyModule_AddIntMacro(m, I_STR)) return -1;
    if (PyModule_AddIntMacro(m, I_SWROPT)) return -1;
#ifdef I_GWROPT
    /* despite the comment above, old-ish glibcs miss a couple... */
    if (PyModule_AddIntMacro(m, I_GWROPT)) return -1;
#endif
    if (PyModule_AddIntMacro(m, I_SENDFD)) return -1;
    if (PyModule_AddIntMacro(m, I_RECVFD)) return -1;
    if (PyModule_AddIntMacro(m, I_LIST)) return -1;
    if (PyModule_AddIntMacro(m, I_ATMARK)) return -1;
    if (PyModule_AddIntMacro(m, I_CKBAND)) return -1;
    if (PyModule_AddIntMacro(m, I_GETBAND)) return -1;
    if (PyModule_AddIntMacro(m, I_CANPUT)) return -1;
    if (PyModule_AddIntMacro(m, I_SETCLTIME)) return -1;
#ifdef I_GETCLTIME
    if (PyModule_AddIntMacro(m, I_GETCLTIME)) return -1;
#endif
    if (PyModule_AddIntMacro(m, I_LINK)) return -1;
    if (PyModule_AddIntMacro(m, I_UNLINK)) return -1;
    if (PyModule_AddIntMacro(m, I_PLINK)) return -1;
    if (PyModule_AddIntMacro(m, I_PUNLINK)) return -1;
#endif

    return 0;
}


static struct PyModuleDef fcntlmodule = {
    PyModuleDef_HEAD_INIT,
    "fcntl",
    module_doc,
    -1,
    fcntl_methods,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit_fcntl(void)
{
    PyObject *m;

    /* Create the module and add the functions and documentation */
    m = PyModule_Create(&fcntlmodule);
    if (m == NULL)
        return NULL;

    /* Add some symbolic constants to the module */
    if (all_ins(m) < 0)
        return NULL;

    return m;
}
