.. highlightlang:: none

.. ATTENTION: You probably should update Misc/python.man, too, if you modify
.. this file.

.. _using-on-general:

Command line and environment
============================

The CPython interpreter scans the command line and the environment for various
settings.

.. impl-detail::

   Other implementations' command line schemes may differ.  See
   :ref:`implementations` for further resources.


.. _using-on-cmdline:

Command line
------------

When invoking Python, you may specify any of these options::

    python [-bBdEhiIOqsSuvVWx?] [-c command | -m module-name | script | - ] [args]

The most common use case is, of course, a simple invocation of a script::

    python myscript.py


.. _using-on-interface-options:

Interface options
~~~~~~~~~~~~~~~~~

The interpreter interface resembles that of the UNIX shell, but provides some
additional methods of invocation:

* When called with standard input connected to a tty device, it prompts for
  commands and executes them until an EOF (an end-of-file character, you can
  produce that with *Ctrl-D* on UNIX or *Ctrl-Z, Enter* on Windows) is read.
* When called with a file name argument or with a file as standard input, it
  reads and executes a script from that file.
* When called with a directory name argument, it reads and executes an
  appropriately named script from that directory.
* When called with ``-c command``, it executes the Python statement(s) given as
  *command*.  Here *command* may contain multiple statements separated by
  newlines. Leading whitespace is significant in Python statements!
* When called with ``-m module-name``, the given module is located on the
  Python module path and executed as a script.

In non-interactive mode, the entire input is parsed before it is executed.

An interface option terminates the list of options consumed by the interpreter,
all consecutive arguments will end up in :data:`sys.argv` -- note that the first
element, subscript zero (``sys.argv[0]``), is a string reflecting the program's
source.

.. cmdoption:: -c <command>

   Execute the Python code in *command*.  *command* can be one or more
   statements separated by newlines, with significant leading whitespace as in
   normal module code.

   If this option is given, the first element of :data:`sys.argv` will be
   ``"-c"`` and the current directory will be added to the start of
   :data:`sys.path` (allowing modules in that directory to be imported as top
   level modules).


.. cmdoption:: -m <module-name>

   Search :data:`sys.path` for the named module and execute its contents as
   the :mod:`__main__` module.

   Since the argument is a *module* name, you must not give a file extension
   (``.py``).  The ``module-name`` should be a valid Python module name, but
   the implementation may not always enforce this (e.g. it may allow you to
   use a name that includes a hyphen).

   Package names (including namespace packages) are also permitted. When a
   package name is supplied instead
   of a normal module, the interpreter will execute ``<pkg>.__main__`` as
   the main module. This behaviour is deliberately similar to the handling
   of directories and zipfiles that are passed to the interpreter as the
   script argument.

   .. note::

      This option cannot be used with built-in modules and extension modules
      written in C, since they do not have Python module files. However, it
      can still be used for precompiled modules, even if the original source
      file is not available.

   If this option is given, the first element of :data:`sys.argv` will be the
   full path to the module file (while the module file is being located, the
   first element will be set to ``"-m"``). As with the :option:`-c` option,
   the current directory will be added to the start of :data:`sys.path`.

   Many standard library modules contain code that is invoked on their execution
   as a script.  An example is the :mod:`timeit` module::

       python -mtimeit -s 'setup here' 'benchmarked code here'
       python -mtimeit -h # for details

   .. seealso::
      :func:`runpy.run_module`
         Equivalent functionality directly available to Python code

      :pep:`338` -- Executing modules as scripts


   .. versionchanged:: 3.1
      Supply the package name to run a ``__main__`` submodule.

   .. versionchanged:: 3.4
      namespace packages are also supported


.. describe:: -

   Read commands from standard input (:data:`sys.stdin`).  If standard input is
   a terminal, :option:`-i` is implied.

   If this option is given, the first element of :data:`sys.argv` will be
   ``"-"`` and the current directory will be added to the start of
   :data:`sys.path`.


.. describe:: <script>

   Execute the Python code contained in *script*, which must be a filesystem
   path (absolute or relative) referring to either a Python file, a directory
   containing a ``__main__.py`` file, or a zipfile containing a
   ``__main__.py`` file.

   If this option is given, the first element of :data:`sys.argv` will be the
   script name as given on the command line.

   If the script name refers directly to a Python file, the directory
   containing that file is added to the start of :data:`sys.path`, and the
   file is executed as the :mod:`__main__` module.

   If the script name refers to a directory or zipfile, the script name is
   added to the start of :data:`sys.path` and the ``__main__.py`` file in
   that location is executed as the :mod:`__main__` module.


If no interface option is given, :option:`-i` is implied, ``sys.argv[0]`` is
an empty string (``""``) and the current directory will be added to the
start of :data:`sys.path`.  Also, tab-completion and history editing is
automatically enabled, if available on your platform (see
:ref:`rlcompleter-config`).

.. versionchanged:: 3.4
   Automatic enabling of tab-completion and history editing.

.. seealso::  :ref:`tut-invoking`


Generic options
~~~~~~~~~~~~~~~

.. cmdoption:: -?
               -h
               --help

   Print a short description of all command line options.


.. cmdoption:: -V
               --version

   Print the Python version number and exit.  Example output could be::

       Python 3.0


.. _using-on-misc-options:

Miscellaneous options
~~~~~~~~~~~~~~~~~~~~~

.. cmdoption:: -b

   Issue a warning when comparing str and bytes. Issue an error when the
   option is given twice (:option:`-bb`).


.. cmdoption:: -B

   If given, Python won't try to write ``.pyc`` or ``.pyo`` files on the
   import of source modules.  See also :envvar:`PYTHONDONTWRITEBYTECODE`.


.. cmdoption:: -d

   Turn on parser debugging output (for wizards only, depending on compilation
   options).  See also :envvar:`PYTHONDEBUG`.


.. cmdoption:: -E

   Ignore all :envvar:`PYTHON*` environment variables, e.g.
   :envvar:`PYTHONPATH` and :envvar:`PYTHONHOME`, that might be set.


.. cmdoption:: -i

   When a script is passed as first argument or the :option:`-c` option is used,
   enter interactive mode after executing the script or the command, even when
   :data:`sys.stdin` does not appear to be a terminal.  The
   :envvar:`PYTHONSTARTUP` file is not read.

   This can be useful to inspect global variables or a stack trace when a script
   raises an exception.  See also :envvar:`PYTHONINSPECT`.


.. cmdoption:: -I

   Run Python in isolated mode. This also implies -E and -s.
   In isolated mode :data:`sys.path` contains neither the script's directory nor
   the user's site-packages directory. All :envvar:`PYTHON*` environment
   variables are ignored, too. Further restrictions may be imposed to prevent
   the user from injecting malicious code.

   .. versionadded:: 3.4


.. cmdoption:: -O

   Turn on basic optimizations.  This changes the filename extension for
   compiled (:term:`bytecode`) files from ``.pyc`` to ``.pyo``.  See also
   :envvar:`PYTHONOPTIMIZE`.


.. cmdoption:: -OO

   Discard docstrings in addition to the :option:`-O` optimizations.


.. cmdoption:: -q

   Don't display the copyright and version messages even in interactive mode.

   .. versionadded:: 3.2


.. cmdoption:: -R

   Kept for compatibility.  On Python 3.3 and greater, hash randomization is
   turned on by default.

   On previous versions of Python, this option turns on hash randomization,
   so that the :meth:`__hash__` values of str, bytes and datetime
   are "salted" with an unpredictable random value.  Although they remain
   constant within an individual Python process, they are not predictable
   between repeated invocations of Python.

   Hash randomization is intended to provide protection against a
   denial-of-service caused by carefully-chosen inputs that exploit the worst
   case performance of a dict construction, O(n^2) complexity.  See
   http://www.ocert.org/advisories/ocert-2011-003.html for details.

   :envvar:`PYTHONHASHSEED` allows you to set a fixed value for the hash
   seed secret.

   .. versionadded:: 3.2.3


.. cmdoption:: -s

   Don't add the :data:`user site-packages directory <site.USER_SITE>` to
   :data:`sys.path`.

   .. seealso::

      :pep:`370` -- Per user site-packages directory


.. cmdoption:: -S

   Disable the import of the module :mod:`site` and the site-dependent
   manipulations of :data:`sys.path` that it entails.  Also disable these
   manipulations if :mod:`site` is explicitly imported later (call
   :func:`site.main` if you want them to be triggered).


.. cmdoption:: -u

   Force the binary layer of the stdout and stderr streams (which is
   available as their ``buffer`` attribute) to be unbuffered. The text I/O
   layer will still be line-buffered if writing to the console, or
   block-buffered if redirected to a non-interactive file.

   See also :envvar:`PYTHONUNBUFFERED`.


.. cmdoption:: -v

   Print a message each time a module is initialized, showing the place
   (filename or built-in module) from which it is loaded.  When given twice
   (:option:`-vv`), print a message for each file that is checked for when
   searching for a module.  Also provides information on module cleanup at exit.
   See also :envvar:`PYTHONVERBOSE`.


.. cmdoption:: -W arg

   Warning control.  Python's warning machinery by default prints warning
   messages to :data:`sys.stderr`.  A typical warning message has the following
   form::

       file:line: category: message

   By default, each warning is printed once for each source line where it
   occurs.  This option controls how often warnings are printed.

   Multiple :option:`-W` options may be given; when a warning matches more than
   one option, the action for the last matching option is performed.  Invalid
   :option:`-W` options are ignored (though, a warning message is printed about
   invalid options when the first warning is issued).

   Warnings can also be controlled from within a Python program using the
   :mod:`warnings` module.

   The simplest form of argument is one of the following action strings (or a
   unique abbreviation):

   ``ignore``
      Ignore all warnings.
   ``default``
      Explicitly request the default behavior (printing each warning once per
      source line).
   ``all``
      Print a warning each time it occurs (this may generate many messages if a
      warning is triggered repeatedly for the same source line, such as inside a
      loop).
   ``module``
      Print each warning only the first time it occurs in each module.
   ``once``
      Print each warning only the first time it occurs in the program.
   ``error``
      Raise an exception instead of printing a warning message.

   The full form of argument is::

       action:message:category:module:line

   Here, *action* is as explained above but only applies to messages that match
   the remaining fields.  Empty fields match all values; trailing empty fields
   may be omitted.  The *message* field matches the start of the warning message
   printed; this match is case-insensitive.  The *category* field matches the
   warning category.  This must be a class name; the match tests whether the
   actual warning category of the message is a subclass of the specified warning
   category.  The full class name must be given.  The *module* field matches the
   (fully-qualified) module name; this match is case-sensitive.  The *line*
   field matches the line number, where zero matches all line numbers and is
   thus equivalent to an omitted line number.

   .. seealso::
      :mod:`warnings` -- the warnings module

      :pep:`230` -- Warning framework

      :envvar:`PYTHONWARNINGS`


.. cmdoption:: -x

   Skip the first line of the source, allowing use of non-Unix forms of
   ``#!cmd``.  This is intended for a DOS specific hack only.

   .. note:: The line numbers in error messages will be off by one.


.. cmdoption:: -X

   Reserved for various implementation-specific options.  CPython currently
   defines the following possible values:

   * ``-X faulthandler`` to enable :mod:`faulthandler`;
   * ``-X showrefcount`` to enable the output of the total reference count
     and memory blocks (only works on debug builds);
   * ``-X tracemalloc`` to start tracing Python memory allocations using the
     :mod:`tracemalloc` module. By default, only the most recent frame is
     stored in a traceback of a trace. Use ``-X tracemalloc=NFRAME`` to start
     tracing with a traceback limit of *NFRAME* frames. See the
     :func:`tracemalloc.start` for more information.

   It also allows to pass arbitrary values and retrieve them through the
   :data:`sys._xoptions` dictionary.

   .. versionchanged:: 3.2
      It is now allowed to pass :option:`-X` with CPython.

   .. versionadded:: 3.3
      The ``-X faulthandler`` option.

   .. versionadded:: 3.4
      The ``-X showrefcount`` and ``-X tracemalloc`` options.


Options you shouldn't use
~~~~~~~~~~~~~~~~~~~~~~~~~

.. cmdoption:: -J

   Reserved for use by Jython_.

.. _Jython: http://jython.org


.. _using-on-envvars:

Environment variables
---------------------

These environment variables influence Python's behavior, they are processed
before the command-line switches other than -E or -I.  It is customary that
command-line switches override environmental variables where there is a
conflict.

.. envvar:: PYTHONHOME

   Change the location of the standard Python libraries.  By default, the
   libraries are searched in :file:`{prefix}/lib/python{version}` and
   :file:`{exec_prefix}/lib/python{version}`, where :file:`{prefix}` and
   :file:`{exec_prefix}` are installation-dependent directories, both defaulting
   to :file:`/usr/local`.

   When :envvar:`PYTHONHOME` is set to a single directory, its value replaces
   both :file:`{prefix}` and :file:`{exec_prefix}`.  To specify different values
   for these, set :envvar:`PYTHONHOME` to :file:`{prefix}:{exec_prefix}`.


.. envvar:: PYTHONPATH

   Augment the default search path for module files.  The format is the same as
   the shell's :envvar:`PATH`: one or more directory pathnames separated by
   :data:`os.pathsep` (e.g. colons on Unix or semicolons on Windows).
   Non-existent directories are silently ignored.

   In addition to normal directories, individual :envvar:`PYTHONPATH` entries
   may refer to zipfiles containing pure Python modules (in either source or
   compiled form). Extension modules cannot be imported from zipfiles.

   The default search path is installation dependent, but generally begins with
   :file:`{prefix}/lib/python{version}` (see :envvar:`PYTHONHOME` above).  It
   is *always* appended to :envvar:`PYTHONPATH`.

   An additional directory will be inserted in the search path in front of
   :envvar:`PYTHONPATH` as described above under
   :ref:`using-on-interface-options`. The search path can be manipulated from
   within a Python program as the variable :data:`sys.path`.


.. envvar:: PYTHONSTARTUP

   If this is the name of a readable file, the Python commands in that file are
   executed before the first prompt is displayed in interactive mode.  The file
   is executed in the same namespace where interactive commands are executed so
   that objects defined or imported in it can be used without qualification in
   the interactive session.  You can also change the prompts :data:`sys.ps1` and
   :data:`sys.ps2` and the hook :data:`sys.__interactivehook__` in this file.


.. envvar:: PYTHONY2K

   Set this to a non-empty string to cause the :mod:`time` module to require
   dates specified as strings to include 4-digit years, otherwise 2-digit years
   are converted based on rules described in the :mod:`time` module
   documentation.


.. envvar:: PYTHONOPTIMIZE

   If this is set to a non-empty string it is equivalent to specifying the
   :option:`-O` option.  If set to an integer, it is equivalent to specifying
   :option:`-O` multiple times.


.. envvar:: PYTHONDEBUG

   If this is set to a non-empty string it is equivalent to specifying the
   :option:`-d` option.  If set to an integer, it is equivalent to specifying
   :option:`-d` multiple times.


.. envvar:: PYTHONINSPECT

   If this is set to a non-empty string it is equivalent to specifying the
   :option:`-i` option.

   This variable can also be modified by Python code using :data:`os.environ`
   to force inspect mode on program termination.


.. envvar:: PYTHONUNBUFFERED

   If this is set to a non-empty string it is equivalent to specifying the
   :option:`-u` option.


.. envvar:: PYTHONVERBOSE

   If this is set to a non-empty string it is equivalent to specifying the
   :option:`-v` option.  If set to an integer, it is equivalent to specifying
   :option:`-v` multiple times.


.. envvar:: PYTHONCASEOK

   If this is set, Python ignores case in :keyword:`import` statements.  This
   only works on Windows and OS X.


.. envvar:: PYTHONDONTWRITEBYTECODE

   If this is set to a non-empty string, Python won't try to write ``.pyc`` or
   ``.pyo`` files on the import of source modules.  This is equivalent to
   specifying the :option:`-B` option.


.. envvar:: PYTHONHASHSEED

   If this variable is not set or set to ``random``, a random value is used
   to seed the hashes of str, bytes and datetime objects.

   If :envvar:`PYTHONHASHSEED` is set to an integer value, it is used as a fixed
   seed for generating the hash() of the types covered by the hash
   randomization.

   Its purpose is to allow repeatable hashing, such as for selftests for the
   interpreter itself, or to allow a cluster of python processes to share hash
   values.

   The integer must be a decimal number in the range [0,4294967295].  Specifying
   the value 0 will disable hash randomization.

   .. versionadded:: 3.2.3


.. envvar:: PYTHONIOENCODING

   If this is set before running the interpreter, it overrides the encoding used
   for stdin/stdout/stderr, in the syntax ``encodingname:errorhandler``.  Both
   the ``encodingname`` and the ``:errorhandler`` parts are optional and have
   the same meaning as in :func:`str.encode`.

   For stderr, the ``:errorhandler`` part is ignored; the handler will always be
   ``'backslashreplace'``.

   .. versionchanged:: 3.4
      The ``encodingname`` part is now optional.


.. envvar:: PYTHONNOUSERSITE

   If this is set, Python won't add the :data:`user site-packages directory
   <site.USER_SITE>` to :data:`sys.path`.

   .. seealso::

      :pep:`370` -- Per user site-packages directory


.. envvar:: PYTHONUSERBASE

   Defines the :data:`user base directory <site.USER_BASE>`, which is used to
   compute the path of the :data:`user site-packages directory <site.USER_SITE>`
   and :ref:`Distutils installation paths <inst-alt-install-user>` for
   ``python setup.py install --user``.

   .. seealso::

      :pep:`370` -- Per user site-packages directory


.. envvar:: PYTHONEXECUTABLE

   If this environment variable is set, ``sys.argv[0]`` will be set to its
   value instead of the value got through the C runtime.  Only works on
   Mac OS X.

.. envvar:: PYTHONWARNINGS

   This is equivalent to the :option:`-W` option. If set to a comma
   separated string, it is equivalent to specifying :option:`-W` multiple
   times.

.. envvar:: PYTHONFAULTHANDLER

   If this environment variable is set to a non-empty string,
   :func:`faulthandler.enable` is called at startup: install a handler for
   :const:`SIGSEGV`, :const:`SIGFPE`, :const:`SIGABRT`, :const:`SIGBUS` and
   :const:`SIGILL` signals to dump the Python traceback.  This is equivalent to
   :option:`-X` ``faulthandler`` option.

   .. versionadded:: 3.3


.. envvar:: PYTHONTRACEMALLOC

   If this environment variable is set to a non-empty string, start tracing
   Python memory allocations using the :mod:`tracemalloc` module. The value of
   the variable is the maximum number of frames stored in a traceback of a
   trace. For example, ``PYTHONTRACEMALLOC=1`` stores only the most recent
   frame. See the :func:`tracemalloc.start` for more information.

   .. versionadded:: 3.4


.. envvar:: PYTHONASYNCIODEBUG

   If this environment variable is set to a non-empty string, enable the debug
   mode of the :mod:`asyncio` module.

   .. versionadded:: 3.4


Debug-mode variables
~~~~~~~~~~~~~~~~~~~~

Setting these variables only has an effect in a debug build of Python, that is,
if Python was configured with the ``--with-pydebug`` build option.

.. envvar:: PYTHONTHREADDEBUG

   If set, Python will print threading debug info.


.. envvar:: PYTHONDUMPREFS

   If set, Python will dump objects and reference counts still alive after
   shutting down the interpreter.


.. envvar:: PYTHONMALLOCSTATS

   If set, Python will print memory allocation statistics every time a new
   object arena is created, and on shutdown.
