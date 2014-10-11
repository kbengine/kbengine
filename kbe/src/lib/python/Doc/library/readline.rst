:mod:`readline` --- GNU readline interface
==========================================

.. module:: readline
   :platform: Unix
   :synopsis: GNU readline support for Python.
.. sectionauthor:: Skip Montanaro <skip@pobox.com>


The :mod:`readline` module defines a number of functions to facilitate
completion and reading/writing of history files from the Python interpreter.
This module can be used directly or via the :mod:`rlcompleter` module.  Settings
made using  this module affect the behaviour of both the interpreter's
interactive prompt  and the prompts offered by the built-in :func:`input`
function.

.. note::

  On MacOS X the :mod:`readline` module can be implemented using
  the ``libedit`` library instead of GNU readline.

  The configuration file for ``libedit`` is different from that
  of GNU readline. If you programmatically load configuration strings
  you can check for the text "libedit" in :const:`readline.__doc__`
  to differentiate between GNU readline and libedit.


The :mod:`readline` module defines the following functions:


.. function:: parse_and_bind(string)

   Parse and execute single line of a readline init file.


.. function:: get_line_buffer()

   Return the current contents of the line buffer.


.. function:: insert_text(string)

   Insert text into the command line.


.. function:: read_init_file([filename])

   Parse a readline initialization file. The default filename is the last filename
   used.


.. function:: read_history_file([filename])

   Load a readline history file. The default filename is :file:`~/.history`.


.. function:: write_history_file([filename])

   Save a readline history file. The default filename is :file:`~/.history`.


.. function:: clear_history()

   Clear the current history.  (Note: this function is not available if the
   installed version of GNU readline doesn't support it.)


.. function:: get_history_length()

   Return the desired length of the history file.  Negative values imply unlimited
   history file size.


.. function:: set_history_length(length)

   Set the number of lines to save in the history file. :func:`write_history_file`
   uses this value to truncate the history file when saving.  Negative values imply
   unlimited history file size.


.. function:: get_current_history_length()

   Return the number of lines currently in the history.  (This is different from
   :func:`get_history_length`, which returns the maximum number of lines that will
   be written to a history file.)


.. function:: get_history_item(index)

   Return the current contents of history item at *index*.


.. function:: remove_history_item(pos)

   Remove history item specified by its position from the history.


.. function:: replace_history_item(pos, line)

   Replace history item specified by its position with the given line.


.. function:: redisplay()

   Change what's displayed on the screen to reflect the current contents of the
   line buffer.


.. function:: set_startup_hook([function])

   Set or remove the startup_hook function.  If *function* is specified, it will be
   used as the new startup_hook function; if omitted or ``None``, any hook function
   already installed is removed.  The startup_hook function is called with no
   arguments just before readline prints the first prompt.


.. function:: set_pre_input_hook([function])

   Set or remove the pre_input_hook function.  If *function* is specified, it will
   be used as the new pre_input_hook function; if omitted or ``None``, any hook
   function already installed is removed.  The pre_input_hook function is called
   with no arguments after the first prompt has been printed and just before
   readline starts reading input characters.


.. function:: set_completer([function])

   Set or remove the completer function.  If *function* is specified, it will be
   used as the new completer function; if omitted or ``None``, any completer
   function already installed is removed.  The completer function is called as
   ``function(text, state)``, for *state* in ``0``, ``1``, ``2``, ..., until it
   returns a non-string value.  It should return the next possible completion
   starting with *text*.


.. function:: get_completer()

   Get the completer function, or ``None`` if no completer function has been set.


.. function:: get_completion_type()

   Get the type of completion being attempted.


.. function:: get_begidx()

   Get the beginning index of the readline tab-completion scope.


.. function:: get_endidx()

   Get the ending index of the readline tab-completion scope.


.. function:: set_completer_delims(string)

   Set the readline word delimiters for tab-completion.


.. function:: get_completer_delims()

   Get the readline word delimiters for tab-completion.


.. function:: set_completion_display_matches_hook([function])

   Set or remove the completion display function.  If *function* is
   specified, it will be used as the new completion display function;
   if omitted or ``None``, any completion display function already
   installed is removed.  The completion display function is called as
   ``function(substitution, [matches], longest_match_length)`` once
   each time matches need to be displayed.


.. function:: add_history(line)

   Append a line to the history buffer, as if it was the last line typed.

.. seealso::

   Module :mod:`rlcompleter`
      Completion of Python identifiers at the interactive prompt.


.. _readline-example:

Example
-------

The following example demonstrates how to use the :mod:`readline` module's
history reading and writing functions to automatically load and save a history
file named :file:`.python_history` from the user's home directory.  The code
below would normally be executed automatically during interactive sessions
from the user's :envvar:`PYTHONSTARTUP` file. ::

   import atexit
   import os
   import readline

   histfile = os.path.join(os.path.expanduser("~"), ".python_history")
   try:
       readline.read_history_file(histfile)
   except FileNotFoundError:
       pass

   atexit.register(readline.write_history_file, histfile)

This code is actually automatically run when Python is run in
:ref:`interactive mode <tut-interactive>` (see :ref:`rlcompleter-config`).

The following example extends the :class:`code.InteractiveConsole` class to
support history save/restore. ::

   import atexit
   import code
   import os
   import readline

   class HistoryConsole(code.InteractiveConsole):
       def __init__(self, locals=None, filename="<console>",
                    histfile=os.path.expanduser("~/.console-history")):
           code.InteractiveConsole.__init__(self, locals, filename)
           self.init_history(histfile)

       def init_history(self, histfile):
           readline.parse_and_bind("tab: complete")
           if hasattr(readline, "read_history_file"):
               try:
                   readline.read_history_file(histfile)
               except FileNotFoundError:
                   pass
               atexit.register(self.save_history, histfile)

       def save_history(self, histfile):
           readline.write_history_file(histfile)

