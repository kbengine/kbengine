"""Temporary files.

This module provides generic, low- and high-level interfaces for
creating temporary files and directories.  The interfaces listed
as "safe" just below can be used without fear of race conditions.
Those listed as "unsafe" cannot, and are provided for backward
compatibility only.

This module also provides some data items to the user:

  TMP_MAX  - maximum number of names that will be tried before
             giving up.
  tempdir  - If this is set to a string before the first use of
             any routine from this module, it will be considered as
             another candidate location to store temporary files.
"""

__all__ = [
    "NamedTemporaryFile", "TemporaryFile", # high level safe interfaces
    "SpooledTemporaryFile", "TemporaryDirectory",
    "mkstemp", "mkdtemp",                  # low level safe interfaces
    "mktemp",                              # deprecated unsafe interface
    "TMP_MAX", "gettempprefix",            # constants
    "tempdir", "gettempdir"
   ]


# Imports.

import functools as _functools
import warnings as _warnings
import io as _io
import os as _os
import shutil as _shutil
import errno as _errno
from random import Random as _Random
import weakref as _weakref

try:
    import _thread
except ImportError:
    import _dummy_thread as _thread
_allocate_lock = _thread.allocate_lock

_text_openflags = _os.O_RDWR | _os.O_CREAT | _os.O_EXCL
if hasattr(_os, 'O_NOFOLLOW'):
    _text_openflags |= _os.O_NOFOLLOW

_bin_openflags = _text_openflags
if hasattr(_os, 'O_BINARY'):
    _bin_openflags |= _os.O_BINARY

if hasattr(_os, 'TMP_MAX'):
    TMP_MAX = _os.TMP_MAX
else:
    TMP_MAX = 10000

# Although it does not have an underscore for historical reasons, this
# variable is an internal implementation detail (see issue 10354).
template = "tmp"

# Internal routines.

_once_lock = _allocate_lock()

if hasattr(_os, "lstat"):
    _stat = _os.lstat
elif hasattr(_os, "stat"):
    _stat = _os.stat
else:
    # Fallback.  All we need is something that raises OSError if the
    # file doesn't exist.
    def _stat(fn):
        fd = _os.open(fn, _os.O_RDONLY)
        _os.close(fd)

def _exists(fn):
    try:
        _stat(fn)
    except OSError:
        return False
    else:
        return True

class _RandomNameSequence:
    """An instance of _RandomNameSequence generates an endless
    sequence of unpredictable strings which can safely be incorporated
    into file names.  Each string is six characters long.  Multiple
    threads can safely use the same instance at the same time.

    _RandomNameSequence is an iterator."""

    characters = "abcdefghijklmnopqrstuvwxyz0123456789_"

    @property
    def rng(self):
        cur_pid = _os.getpid()
        if cur_pid != getattr(self, '_rng_pid', None):
            self._rng = _Random()
            self._rng_pid = cur_pid
        return self._rng

    def __iter__(self):
        return self

    def __next__(self):
        c = self.characters
        choose = self.rng.choice
        letters = [choose(c) for dummy in range(8)]
        return ''.join(letters)

def _candidate_tempdir_list():
    """Generate a list of candidate temporary directories which
    _get_default_tempdir will try."""

    dirlist = []

    # First, try the environment.
    for envname in 'TMPDIR', 'TEMP', 'TMP':
        dirname = _os.getenv(envname)
        if dirname: dirlist.append(dirname)

    # Failing that, try OS-specific locations.
    if _os.name == 'nt':
        dirlist.extend([ r'c:\temp', r'c:\tmp', r'\temp', r'\tmp' ])
    else:
        dirlist.extend([ '/tmp', '/var/tmp', '/usr/tmp' ])

    # As a last resort, the current directory.
    try:
        dirlist.append(_os.getcwd())
    except (AttributeError, OSError):
        dirlist.append(_os.curdir)

    return dirlist

def _get_default_tempdir():
    """Calculate the default directory to use for temporary files.
    This routine should be called exactly once.

    We determine whether or not a candidate temp dir is usable by
    trying to create and write to a file in that directory.  If this
    is successful, the test file is deleted.  To prevent denial of
    service, the name of the test file must be randomized."""

    namer = _RandomNameSequence()
    dirlist = _candidate_tempdir_list()

    for dir in dirlist:
        if dir != _os.curdir:
            dir = _os.path.abspath(dir)
        # Try only a few names per directory.
        for seq in range(100):
            name = next(namer)
            filename = _os.path.join(dir, name)
            try:
                fd = _os.open(filename, _bin_openflags, 0o600)
                try:
                    try:
                        with _io.open(fd, 'wb', closefd=False) as fp:
                            fp.write(b'blat')
                    finally:
                        _os.close(fd)
                finally:
                    _os.unlink(filename)
                return dir
            except FileExistsError:
                pass
            except OSError:
                break   # no point trying more names in this directory
    raise FileNotFoundError(_errno.ENOENT,
                            "No usable temporary directory found in %s" %
                            dirlist)

_name_sequence = None

def _get_candidate_names():
    """Common setup sequence for all user-callable interfaces."""

    global _name_sequence
    if _name_sequence is None:
        _once_lock.acquire()
        try:
            if _name_sequence is None:
                _name_sequence = _RandomNameSequence()
        finally:
            _once_lock.release()
    return _name_sequence


def _mkstemp_inner(dir, pre, suf, flags):
    """Code common to mkstemp, TemporaryFile, and NamedTemporaryFile."""

    names = _get_candidate_names()

    for seq in range(TMP_MAX):
        name = next(names)
        file = _os.path.join(dir, pre + name + suf)
        try:
            fd = _os.open(file, flags, 0o600)
            return (fd, _os.path.abspath(file))
        except FileExistsError:
            continue    # try again
        except PermissionError:
            # This exception is thrown when a directory with the chosen name
            # already exists on windows.
            if _os.name == 'nt':
                continue
            else:
                raise

    raise FileExistsError(_errno.EEXIST,
                          "No usable temporary file name found")


# User visible interfaces.

def gettempprefix():
    """Accessor for tempdir.template."""
    return template

tempdir = None

def gettempdir():
    """Accessor for tempfile.tempdir."""
    global tempdir
    if tempdir is None:
        _once_lock.acquire()
        try:
            if tempdir is None:
                tempdir = _get_default_tempdir()
        finally:
            _once_lock.release()
    return tempdir

def mkstemp(suffix="", prefix=template, dir=None, text=False):
    """User-callable function to create and return a unique temporary
    file.  The return value is a pair (fd, name) where fd is the
    file descriptor returned by os.open, and name is the filename.

    If 'suffix' is specified, the file name will end with that suffix,
    otherwise there will be no suffix.

    If 'prefix' is specified, the file name will begin with that prefix,
    otherwise a default prefix is used.

    If 'dir' is specified, the file will be created in that directory,
    otherwise a default directory is used.

    If 'text' is specified and true, the file is opened in text
    mode.  Else (the default) the file is opened in binary mode.  On
    some operating systems, this makes no difference.

    The file is readable and writable only by the creating user ID.
    If the operating system uses permission bits to indicate whether a
    file is executable, the file is executable by no one. The file
    descriptor is not inherited by children of this process.

    Caller is responsible for deleting the file when done with it.
    """

    if dir is None:
        dir = gettempdir()

    if text:
        flags = _text_openflags
    else:
        flags = _bin_openflags

    return _mkstemp_inner(dir, prefix, suffix, flags)


def mkdtemp(suffix="", prefix=template, dir=None):
    """User-callable function to create and return a unique temporary
    directory.  The return value is the pathname of the directory.

    Arguments are as for mkstemp, except that the 'text' argument is
    not accepted.

    The directory is readable, writable, and searchable only by the
    creating user.

    Caller is responsible for deleting the directory when done with it.
    """

    if dir is None:
        dir = gettempdir()

    names = _get_candidate_names()

    for seq in range(TMP_MAX):
        name = next(names)
        file = _os.path.join(dir, prefix + name + suffix)
        try:
            _os.mkdir(file, 0o700)
            return file
        except FileExistsError:
            continue    # try again

    raise FileExistsError(_errno.EEXIST,
                          "No usable temporary directory name found")

def mktemp(suffix="", prefix=template, dir=None):
    """User-callable function to return a unique temporary file name.  The
    file is not created.

    Arguments are as for mkstemp, except that the 'text' argument is
    not accepted.

    This function is unsafe and should not be used.  The file name
    refers to a file that did not exist at some point, but by the time
    you get around to creating it, someone else may have beaten you to
    the punch.
    """

##    from warnings import warn as _warn
##    _warn("mktemp is a potential security risk to your program",
##          RuntimeWarning, stacklevel=2)

    if dir is None:
        dir = gettempdir()

    names = _get_candidate_names()
    for seq in range(TMP_MAX):
        name = next(names)
        file = _os.path.join(dir, prefix + name + suffix)
        if not _exists(file):
            return file

    raise FileExistsError(_errno.EEXIST,
                          "No usable temporary filename found")


class _TemporaryFileCloser:
    """A separate object allowing proper closing of a temporary file's
    underlying file object, without adding a __del__ method to the
    temporary file."""

    file = None  # Set here since __del__ checks it
    close_called = False

    def __init__(self, file, name, delete=True):
        self.file = file
        self.name = name
        self.delete = delete

    # NT provides delete-on-close as a primitive, so we don't need
    # the wrapper to do anything special.  We still use it so that
    # file.name is useful (i.e. not "(fdopen)") with NamedTemporaryFile.
    if _os.name != 'nt':
        # Cache the unlinker so we don't get spurious errors at
        # shutdown when the module-level "os" is None'd out.  Note
        # that this must be referenced as self.unlink, because the
        # name TemporaryFileWrapper may also get None'd out before
        # __del__ is called.

        def close(self, unlink=_os.unlink):
            if not self.close_called and self.file is not None:
                self.close_called = True
                self.file.close()
                if self.delete:
                    unlink(self.name)

        # Need to ensure the file is deleted on __del__
        def __del__(self):
            self.close()

    else:
        def close(self):
            if not self.close_called:
                self.close_called = True
                self.file.close()


class _TemporaryFileWrapper:
    """Temporary file wrapper

    This class provides a wrapper around files opened for
    temporary use.  In particular, it seeks to automatically
    remove the file when it is no longer needed.
    """

    def __init__(self, file, name, delete=True):
        self.file = file
        self.name = name
        self.delete = delete
        self._closer = _TemporaryFileCloser(file, name, delete)

    def __getattr__(self, name):
        # Attribute lookups are delegated to the underlying file
        # and cached for non-numeric results
        # (i.e. methods are cached, closed and friends are not)
        file = self.__dict__['file']
        a = getattr(file, name)
        if hasattr(a, '__call__'):
            func = a
            @_functools.wraps(func)
            def func_wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            # Avoid closing the file as long as the wrapper is alive,
            # see issue #18879.
            func_wrapper._closer = self._closer
            a = func_wrapper
        if not isinstance(a, int):
            setattr(self, name, a)
        return a

    # The underlying __enter__ method returns the wrong object
    # (self.file) so override it to return the wrapper
    def __enter__(self):
        self.file.__enter__()
        return self

    # Need to trap __exit__ as well to ensure the file gets
    # deleted when used in a with statement
    def __exit__(self, exc, value, tb):
        result = self.file.__exit__(exc, value, tb)
        self.close()
        return result

    def close(self):
        """
        Close the temporary file, possibly deleting it.
        """
        self._closer.close()

    # iter() doesn't use __getattr__ to find the __iter__ method
    def __iter__(self):
        return iter(self.file)


def NamedTemporaryFile(mode='w+b', buffering=-1, encoding=None,
                       newline=None, suffix="", prefix=template,
                       dir=None, delete=True):
    """Create and return a temporary file.
    Arguments:
    'prefix', 'suffix', 'dir' -- as for mkstemp.
    'mode' -- the mode argument to io.open (default "w+b").
    'buffering' -- the buffer size argument to io.open (default -1).
    'encoding' -- the encoding argument to io.open (default None)
    'newline' -- the newline argument to io.open (default None)
    'delete' -- whether the file is deleted on close (default True).
    The file is created as mkstemp() would do it.

    Returns an object with a file-like interface; the name of the file
    is accessible as file.name.  The file will be automatically deleted
    when it is closed unless the 'delete' argument is set to False.
    """

    if dir is None:
        dir = gettempdir()

    flags = _bin_openflags

    # Setting O_TEMPORARY in the flags causes the OS to delete
    # the file when it is closed.  This is only supported by Windows.
    if _os.name == 'nt' and delete:
        flags |= _os.O_TEMPORARY

    (fd, name) = _mkstemp_inner(dir, prefix, suffix, flags)
    try:
        file = _io.open(fd, mode, buffering=buffering,
                        newline=newline, encoding=encoding)

        return _TemporaryFileWrapper(file, name, delete)
    except Exception:
        _os.close(fd)
        raise

if _os.name != 'posix' or _os.sys.platform == 'cygwin':
    # On non-POSIX and Cygwin systems, assume that we cannot unlink a file
    # while it is open.
    TemporaryFile = NamedTemporaryFile

else:
    def TemporaryFile(mode='w+b', buffering=-1, encoding=None,
                      newline=None, suffix="", prefix=template,
                      dir=None):
        """Create and return a temporary file.
        Arguments:
        'prefix', 'suffix', 'dir' -- as for mkstemp.
        'mode' -- the mode argument to io.open (default "w+b").
        'buffering' -- the buffer size argument to io.open (default -1).
        'encoding' -- the encoding argument to io.open (default None)
        'newline' -- the newline argument to io.open (default None)
        The file is created as mkstemp() would do it.

        Returns an object with a file-like interface.  The file has no
        name, and will cease to exist when it is closed.
        """

        if dir is None:
            dir = gettempdir()

        flags = _bin_openflags

        (fd, name) = _mkstemp_inner(dir, prefix, suffix, flags)
        try:
            _os.unlink(name)
            return _io.open(fd, mode, buffering=buffering,
                            newline=newline, encoding=encoding)
        except:
            _os.close(fd)
            raise

class SpooledTemporaryFile:
    """Temporary file wrapper, specialized to switch from BytesIO
    or StringIO to a real file when it exceeds a certain size or
    when a fileno is needed.
    """
    _rolled = False

    def __init__(self, max_size=0, mode='w+b', buffering=-1,
                 encoding=None, newline=None,
                 suffix="", prefix=template, dir=None):
        if 'b' in mode:
            self._file = _io.BytesIO()
        else:
            # Setting newline="\n" avoids newline translation;
            # this is important because otherwise on Windows we'd
            # hget double newline translation upon rollover().
            self._file = _io.StringIO(newline="\n")
        self._max_size = max_size
        self._rolled = False
        self._TemporaryFileArgs = {'mode': mode, 'buffering': buffering,
                                   'suffix': suffix, 'prefix': prefix,
                                   'encoding': encoding, 'newline': newline,
                                   'dir': dir}

    def _check(self, file):
        if self._rolled: return
        max_size = self._max_size
        if max_size and file.tell() > max_size:
            self.rollover()

    def rollover(self):
        if self._rolled: return
        file = self._file
        newfile = self._file = TemporaryFile(**self._TemporaryFileArgs)
        del self._TemporaryFileArgs

        newfile.write(file.getvalue())
        newfile.seek(file.tell(), 0)

        self._rolled = True

    # The method caching trick from NamedTemporaryFile
    # won't work here, because _file may change from a
    # BytesIO/StringIO instance to a real file. So we list
    # all the methods directly.

    # Context management protocol
    def __enter__(self):
        if self._file.closed:
            raise ValueError("Cannot enter context with closed file")
        return self

    def __exit__(self, exc, value, tb):
        self._file.close()

    # file protocol
    def __iter__(self):
        return self._file.__iter__()

    def close(self):
        self._file.close()

    @property
    def closed(self):
        return self._file.closed

    @property
    def encoding(self):
        try:
            return self._file.encoding
        except AttributeError:
            if 'b' in self._TemporaryFileArgs['mode']:
                raise
            return self._TemporaryFileArgs['encoding']

    def fileno(self):
        self.rollover()
        return self._file.fileno()

    def flush(self):
        self._file.flush()

    def isatty(self):
        return self._file.isatty()

    @property
    def mode(self):
        try:
            return self._file.mode
        except AttributeError:
            return self._TemporaryFileArgs['mode']

    @property
    def name(self):
        try:
            return self._file.name
        except AttributeError:
            return None

    @property
    def newlines(self):
        try:
            return self._file.newlines
        except AttributeError:
            if 'b' in self._TemporaryFileArgs['mode']:
                raise
            return self._TemporaryFileArgs['newline']

    def read(self, *args):
        return self._file.read(*args)

    def readline(self, *args):
        return self._file.readline(*args)

    def readlines(self, *args):
        return self._file.readlines(*args)

    def seek(self, *args):
        self._file.seek(*args)

    @property
    def softspace(self):
        return self._file.softspace

    def tell(self):
        return self._file.tell()

    def truncate(self, size=None):
        if size is None:
            self._file.truncate()
        else:
            if size > self._max_size:
                self.rollover()
            self._file.truncate(size)

    def write(self, s):
        file = self._file
        rv = file.write(s)
        self._check(file)
        return rv

    def writelines(self, iterable):
        file = self._file
        rv = file.writelines(iterable)
        self._check(file)
        return rv


class TemporaryDirectory(object):
    """Create and return a temporary directory.  This has the same
    behavior as mkdtemp but can be used as a context manager.  For
    example:

        with TemporaryDirectory() as tmpdir:
            ...

    Upon exiting the context, the directory and everything contained
    in it are removed.
    """

    # Handle mkdtemp raising an exception
    name = None
    _finalizer = None
    _closed = False

    def __init__(self, suffix="", prefix=template, dir=None):
        self.name = mkdtemp(suffix, prefix, dir)
        self._finalizer = _weakref.finalize(
            self, self._cleanup, self.name,
            warn_message="Implicitly cleaning up {!r}".format(self))

    @classmethod
    def _cleanup(cls, name, warn_message=None):
        _shutil.rmtree(name)
        if warn_message is not None:
            _warnings.warn(warn_message, ResourceWarning)


    def __repr__(self):
        return "<{} {!r}>".format(self.__class__.__name__, self.name)

    def __enter__(self):
        return self.name

    def __exit__(self, exc, value, tb):
        self.cleanup()

    def cleanup(self):
        if self._finalizer is not None:
            self._finalizer.detach()
        if self.name is not None and not self._closed:
            _shutil.rmtree(self.name)
            self._closed = True
