:mod:`shlex` --- Simple lexical analysis
========================================

.. module:: shlex
   :synopsis: Simple lexical analysis for Unix shell-like languages.
.. moduleauthor:: Eric S. Raymond <esr@snark.thyrsus.com>
.. moduleauthor:: Gustavo Niemeyer <niemeyer@conectiva.com>
.. sectionauthor:: Eric S. Raymond <esr@snark.thyrsus.com>
.. sectionauthor:: Gustavo Niemeyer <niemeyer@conectiva.com>

**Source code:** :source:`Lib/shlex.py`

--------------

The :class:`~shlex.shlex` class makes it easy to write lexical analyzers for
simple syntaxes resembling that of the Unix shell.  This will often be useful
for writing minilanguages, (for example, in run control files for Python
applications) or for parsing quoted strings.

The :mod:`shlex` module defines the following functions:


.. function:: split(s, comments=False, posix=True)

   Split the string *s* using shell-like syntax. If *comments* is :const:`False`
   (the default), the parsing of comments in the given string will be disabled
   (setting the :attr:`~shlex.commenters` attribute of the
   :class:`~shlex.shlex` instance to the empty string).  This function operates
   in POSIX mode by default, but uses non-POSIX mode if the *posix* argument is
   false.

   .. note::

      Since the :func:`split` function instantiates a :class:`~shlex.shlex`
      instance, passing ``None`` for *s* will read the string to split from
      standard input.


.. function:: quote(s)

   Return a shell-escaped version of the string *s*.  The returned value is a
   string that can safely be used as one token in a shell command line, for
   cases where you cannot use a list.

   This idiom would be unsafe::

      >>> filename = 'somefile; rm -rf ~'
      >>> command = 'ls -l {}'.format(filename)
      >>> print(command)  # executed by a shell: boom!
      ls -l somefile; rm -rf ~

   :func:`quote` lets you plug the security hole::

      >>> command = 'ls -l {}'.format(quote(filename))
      >>> print(command)
      ls -l 'somefile; rm -rf ~'
      >>> remote_command = 'ssh home {}'.format(quote(command))
      >>> print(remote_command)
      ssh home 'ls -l '"'"'somefile; rm -rf ~'"'"''

   The quoting is compatible with UNIX shells and with :func:`split`:

      >>> remote_command = split(remote_command)
      >>> remote_command
      ['ssh', 'home', "ls -l 'somefile; rm -rf ~'"]
      >>> command = split(remote_command[-1])
      >>> command
      ['ls', '-l', 'somefile; rm -rf ~']

   .. versionadded:: 3.3

The :mod:`shlex` module defines the following class:


.. class:: shlex(instream=None, infile=None, posix=False)

   A :class:`~shlex.shlex` instance or subclass instance is a lexical analyzer
   object.  The initialization argument, if present, specifies where to read
   characters from. It must be a file-/stream-like object with
   :meth:`~io.TextIOBase.read` and :meth:`~io.TextIOBase.readline` methods, or
   a string.  If no argument is given, input will be taken from ``sys.stdin``.
   The second optional argument is a filename string, which sets the initial
   value of the :attr:`~shlex.infile` attribute.  If the *instream*
   argument is omitted or equal to ``sys.stdin``, this second argument
   defaults to "stdin".  The *posix* argument defines the operational mode:
   when *posix* is not true (default), the :class:`~shlex.shlex` instance will
   operate in compatibility mode.  When operating in POSIX mode,
   :class:`~shlex.shlex` will try to be as close as possible to the POSIX shell
   parsing rules.


.. seealso::

   Module :mod:`configparser`
      Parser for configuration files similar to the Windows :file:`.ini` files.


.. _shlex-objects:

shlex Objects
-------------

A :class:`~shlex.shlex` instance has the following methods:


.. method:: shlex.get_token()

   Return a token.  If tokens have been stacked using :meth:`push_token`, pop a
   token off the stack.  Otherwise, read one from the input stream.  If reading
   encounters an immediate end-of-file, :attr:`eof` is returned (the empty
   string (``''``) in non-POSIX mode, and ``None`` in POSIX mode).


.. method:: shlex.push_token(str)

   Push the argument onto the token stack.


.. method:: shlex.read_token()

   Read a raw token.  Ignore the pushback stack, and do not interpret source
   requests.  (This is not ordinarily a useful entry point, and is documented here
   only for the sake of completeness.)


.. method:: shlex.sourcehook(filename)

   When :class:`~shlex.shlex` detects a source request (see :attr:`source`
   below) this method is given the following token as argument, and expected
   to return a tuple consisting of a filename and an open file-like object.

   Normally, this method first strips any quotes off the argument.  If the result
   is an absolute pathname, or there was no previous source request in effect, or
   the previous source was a stream (such as ``sys.stdin``), the result is left
   alone.  Otherwise, if the result is a relative pathname, the directory part of
   the name of the file immediately before it on the source inclusion stack is
   prepended (this behavior is like the way the C preprocessor handles ``#include
   "file.h"``).

   The result of the manipulations is treated as a filename, and returned as the
   first component of the tuple, with :func:`open` called on it to yield the second
   component. (Note: this is the reverse of the order of arguments in instance
   initialization!)

   This hook is exposed so that you can use it to implement directory search paths,
   addition of file extensions, and other namespace hacks. There is no
   corresponding 'close' hook, but a shlex instance will call the
   :meth:`~io.IOBase.close` method of the sourced input stream when it returns
   EOF.

   For more explicit control of source stacking, use the :meth:`push_source` and
   :meth:`pop_source` methods.


.. method:: shlex.push_source(newstream, newfile=None)

   Push an input source stream onto the input stack.  If the filename argument is
   specified it will later be available for use in error messages.  This is the
   same method used internally by the :meth:`sourcehook` method.


.. method:: shlex.pop_source()

   Pop the last-pushed input source from the input stack. This is the same method
   used internally when the lexer reaches EOF on a stacked input stream.


.. method:: shlex.error_leader(infile=None, lineno=None)

   This method generates an error message leader in the format of a Unix C compiler
   error label; the format is ``'"%s", line %d: '``, where the ``%s`` is replaced
   with the name of the current source file and the ``%d`` with the current input
   line number (the optional arguments can be used to override these).

   This convenience is provided to encourage :mod:`shlex` users to generate error
   messages in the standard, parseable format understood by Emacs and other Unix
   tools.

Instances of :class:`~shlex.shlex` subclasses have some public instance
variables which either control lexical analysis or can be used for debugging:


.. attribute:: shlex.commenters

   The string of characters that are recognized as comment beginners. All
   characters from the comment beginner to end of line are ignored. Includes just
   ``'#'`` by default.


.. attribute:: shlex.wordchars

   The string of characters that will accumulate into multi-character tokens.  By
   default, includes all ASCII alphanumerics and underscore.


.. attribute:: shlex.whitespace

   Characters that will be considered whitespace and skipped.  Whitespace bounds
   tokens.  By default, includes space, tab, linefeed and carriage-return.


.. attribute:: shlex.escape

   Characters that will be considered as escape. This will be only used in POSIX
   mode, and includes just ``'\'`` by default.


.. attribute:: shlex.quotes

   Characters that will be considered string quotes.  The token accumulates until
   the same quote is encountered again (thus, different quote types protect each
   other as in the shell.)  By default, includes ASCII single and double quotes.


.. attribute:: shlex.escapedquotes

   Characters in :attr:`quotes` that will interpret escape characters defined in
   :attr:`escape`.  This is only used in POSIX mode, and includes just ``'"'`` by
   default.


.. attribute:: shlex.whitespace_split

   If ``True``, tokens will only be split in whitespaces. This is useful, for
   example, for parsing command lines with :class:`~shlex.shlex`, getting
   tokens in a similar way to shell arguments.


.. attribute:: shlex.infile

   The name of the current input file, as initially set at class instantiation time
   or stacked by later source requests.  It may be useful to examine this when
   constructing error messages.


.. attribute:: shlex.instream

   The input stream from which this :class:`~shlex.shlex` instance is reading
   characters.


.. attribute:: shlex.source

   This attribute is ``None`` by default.  If you assign a string to it, that
   string will be recognized as a lexical-level inclusion request similar to the
   ``source`` keyword in various shells.  That is, the immediately following token
   will opened as a filename and input taken from that stream until EOF, at which
   point the :meth:`~io.IOBase.close` method of that stream will be called and
   the input source will again become the original input stream.  Source
   requests may be stacked any number of levels deep.


.. attribute:: shlex.debug

   If this attribute is numeric and ``1`` or more, a :class:`~shlex.shlex`
   instance will print verbose progress output on its behavior.  If you need
   to use this, you can read the module source code to learn the details.


.. attribute:: shlex.lineno

   Source line number (count of newlines seen so far plus one).


.. attribute:: shlex.token

   The token buffer.  It may be useful to examine this when catching exceptions.


.. attribute:: shlex.eof

   Token used to determine end of file. This will be set to the empty string
   (``''``), in non-POSIX mode, and to ``None`` in POSIX mode.


.. _shlex-parsing-rules:

Parsing Rules
-------------

When operating in non-POSIX mode, :class:`~shlex.shlex` will try to obey to the
following rules.

* Quote characters are not recognized within words (``Do"Not"Separate`` is
  parsed as the single word ``Do"Not"Separate``);

* Escape characters are not recognized;

* Enclosing characters in quotes preserve the literal value of all characters
  within the quotes;

* Closing quotes separate words (``"Do"Separate`` is parsed as ``"Do"`` and
  ``Separate``);

* If :attr:`~shlex.whitespace_split` is ``False``, any character not
  declared to be a word character, whitespace, or a quote will be returned as
  a single-character token. If it is ``True``, :class:`~shlex.shlex` will only
  split words in whitespaces;

* EOF is signaled with an empty string (``''``);

* It's not possible to parse empty strings, even if quoted.

When operating in POSIX mode, :class:`~shlex.shlex` will try to obey to the
following parsing rules.

* Quotes are stripped out, and do not separate words (``"Do"Not"Separate"`` is
  parsed as the single word ``DoNotSeparate``);

* Non-quoted escape characters (e.g. ``'\'``) preserve the literal value of the
  next character that follows;

* Enclosing characters in quotes which are not part of
  :attr:`~shlex.escapedquotes` (e.g. ``"'"``) preserve the literal value
  of all characters within the quotes;

* Enclosing characters in quotes which are part of
  :attr:`~shlex.escapedquotes` (e.g. ``'"'``) preserves the literal value
  of all characters within the quotes, with the exception of the characters
  mentioned in :attr:`~shlex.escape`.  The escape characters retain its
  special meaning only when followed by the quote in use, or the escape
  character itself. Otherwise the escape character will be considered a
  normal character.

* EOF is signaled with a :const:`None` value;

* Quoted empty strings (``''``) are allowed.
