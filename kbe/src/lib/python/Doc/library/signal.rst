:mod:`signal` --- Set handlers for asynchronous events
======================================================

.. module:: signal
   :synopsis: Set handlers for asynchronous events.


This module provides mechanisms to use signal handlers in Python.


General rules
-------------

The :func:`signal.signal` function allows to define custom handlers to be
executed when a signal is received.  A small number of default handlers are
installed: :const:`SIGPIPE` is ignored (so write errors on pipes and sockets
can be reported as ordinary Python exceptions) and :const:`SIGINT` is
translated into a :exc:`KeyboardInterrupt` exception.

A handler for a particular signal, once set, remains installed until it is
explicitly reset (Python emulates the BSD style interface regardless of the
underlying implementation), with the exception of the handler for
:const:`SIGCHLD`, which follows the underlying implementation.

There is no way to "block" signals temporarily from critical sections (since
this is not supported by all Unix flavors).


Execution of Python signal handlers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A Python signal handler does not get executed inside the low-level (C) signal
handler.  Instead, the low-level signal handler sets a flag which tells the
:term:`virtual machine` to execute the corresponding Python signal handler
at a later point(for example at the next :term:`bytecode` instruction).
This has consequences:

* It makes little sense to catch synchronous errors like :const:`SIGFPE` or
  :const:`SIGSEGV` that are caused by an invalid operation in C code.  Python
  will return from the signal handler to the C code, which is likely to raise
  the same signal again, causing Python to apparently hang.  From Python 3.3
  onwards, you can use the :mod:`faulthandler` module to report on synchronous
  errors.

* A long-running calculation implemented purely in C (such as regular
  expression matching on a large body of text) may run uninterrupted for an
  arbitrary amount of time, regardless of any signals received.  The Python
  signal handlers will be called when the calculation finishes.


.. _signals-and-threads:


Signals and threads
^^^^^^^^^^^^^^^^^^^

Python signal handlers are always executed in the main Python thread,
even if the signal was received in another thread.  This means that signals
can't be used as a means of inter-thread communication.  You can use
the synchronization primitives from the :mod:`threading` module instead.

Besides, only the main thread is allowed to set a new signal handler.


Module contents
---------------

The variables defined in the :mod:`signal` module are:


.. data:: SIG_DFL

   This is one of two standard signal handling options; it will simply perform
   the default function for the signal.  For example, on most systems the
   default action for :const:`SIGQUIT` is to dump core and exit, while the
   default action for :const:`SIGCHLD` is to simply ignore it.


.. data:: SIG_IGN

   This is another standard signal handler, which will simply ignore the given
   signal.


.. data:: SIG*

   All the signal numbers are defined symbolically.  For example, the hangup signal
   is defined as :const:`signal.SIGHUP`; the variable names are identical to the
   names used in C programs, as found in ``<signal.h>``. The Unix man page for
   ':c:func:`signal`' lists the existing signals (on some systems this is
   :manpage:`signal(2)`, on others the list is in :manpage:`signal(7)`). Note that
   not all systems define the same set of signal names; only those names defined by
   the system are defined by this module.


.. data:: CTRL_C_EVENT

   The signal corresponding to the CTRL+C keystroke event. This signal can
   only be used with :func:`os.kill`.

   Availability: Windows.

   .. versionadded:: 3.2


.. data:: CTRL_BREAK_EVENT

   The signal corresponding to the CTRL+BREAK keystroke event. This signal can
   only be used with :func:`os.kill`.

   Availability: Windows.

   .. versionadded:: 3.2


.. data:: NSIG

   One more than the number of the highest signal number.


.. data:: ITIMER_REAL

   Decrements interval timer in real time, and delivers :const:`SIGALRM` upon
   expiration.


.. data:: ITIMER_VIRTUAL

   Decrements interval timer only when the process is executing, and delivers
   SIGVTALRM upon expiration.


.. data:: ITIMER_PROF

   Decrements interval timer both when the process executes and when the
   system is executing on behalf of the process. Coupled with ITIMER_VIRTUAL,
   this timer is usually used to profile the time spent by the application
   in user and kernel space. SIGPROF is delivered upon expiration.


.. data:: SIG_BLOCK

   A possible value for the *how* parameter to :func:`pthread_sigmask`
   indicating that signals are to be blocked.

   .. versionadded:: 3.3

.. data:: SIG_UNBLOCK

   A possible value for the *how* parameter to :func:`pthread_sigmask`
   indicating that signals are to be unblocked.

   .. versionadded:: 3.3

.. data:: SIG_SETMASK

   A possible value for the *how* parameter to :func:`pthread_sigmask`
   indicating that the signal mask is to be replaced.

   .. versionadded:: 3.3


The :mod:`signal` module defines one exception:

.. exception:: ItimerError

   Raised to signal an error from the underlying :func:`setitimer` or
   :func:`getitimer` implementation. Expect this error if an invalid
   interval timer or a negative time is passed to :func:`setitimer`.
   This error is a subtype of :exc:`OSError`.

   .. versionadded:: 3.3
      This error used to be a subtype of :exc:`IOError`, which is now an
      alias of :exc:`OSError`.


The :mod:`signal` module defines the following functions:


.. function:: alarm(time)

   If *time* is non-zero, this function requests that a :const:`SIGALRM` signal be
   sent to the process in *time* seconds. Any previously scheduled alarm is
   canceled (only one alarm can be scheduled at any time).  The returned value is
   then the number of seconds before any previously set alarm was to have been
   delivered. If *time* is zero, no alarm is scheduled, and any scheduled alarm is
   canceled.  If the return value is zero, no alarm is currently scheduled.  (See
   the Unix man page :manpage:`alarm(2)`.) Availability: Unix.


.. function:: getsignal(signalnum)

   Return the current signal handler for the signal *signalnum*. The returned value
   may be a callable Python object, or one of the special values
   :const:`signal.SIG_IGN`, :const:`signal.SIG_DFL` or :const:`None`.  Here,
   :const:`signal.SIG_IGN` means that the signal was previously ignored,
   :const:`signal.SIG_DFL` means that the default way of handling the signal was
   previously in use, and ``None`` means that the previous signal handler was not
   installed from Python.


.. function:: pause()

   Cause the process to sleep until a signal is received; the appropriate handler
   will then be called.  Returns nothing.  Not on Windows. (See the Unix man page
   :manpage:`signal(2)`.)

   See also :func:`sigwait`, :func:`sigwaitinfo`, :func:`sigtimedwait` and
   :func:`sigpending`.


.. function:: pthread_kill(thread_id, signum)

   Send the signal *signum* to the thread *thread_id*, another thread in the
   same process as the caller.  The target thread can be executing any code
   (Python or not).  However, if the target thread is executing the Python
   interpreter, the Python signal handlers will be :ref:`executed by the main
   thread <signals-and-threads>`.  Therefore, the only point of sending a signal to a particular
   Python thread would be to force a running system call to fail with
   :exc:`InterruptedError`.

   Use :func:`threading.get_ident()` or the :attr:`~threading.Thread.ident`
   attribute of :class:`threading.Thread` objects to get a suitable value
   for *thread_id*.

   If *signum* is 0, then no signal is sent, but error checking is still
   performed; this can be used to check if the target thread is still running.

   Availability: Unix (see the man page :manpage:`pthread_kill(3)` for further
   information).

   See also :func:`os.kill`.

   .. versionadded:: 3.3


.. function:: pthread_sigmask(how, mask)

   Fetch and/or change the signal mask of the calling thread.  The signal mask
   is the set of signals whose delivery is currently blocked for the caller.
   Return the old signal mask as a set of signals.

   The behavior of the call is dependent on the value of *how*, as follows.

   * :data:`SIG_BLOCK`: The set of blocked signals is the union of the current
     set and the *mask* argument.
   * :data:`SIG_UNBLOCK`: The signals in *mask* are removed from the current
     set of blocked signals.  It is permissible to attempt to unblock a
     signal which is not blocked.
   * :data:`SIG_SETMASK`: The set of blocked signals is set to the *mask*
     argument.

   *mask* is a set of signal numbers (e.g. {:const:`signal.SIGINT`,
   :const:`signal.SIGTERM`}). Use ``range(1, signal.NSIG)`` for a full mask
   including all signals.

   For example, ``signal.pthread_sigmask(signal.SIG_BLOCK, [])`` reads the
   signal mask of the calling thread.

   Availability: Unix. See the man page :manpage:`sigprocmask(3)` and
   :manpage:`pthread_sigmask(3)` for further information.

   See also :func:`pause`, :func:`sigpending` and :func:`sigwait`.

   .. versionadded:: 3.3


.. function:: setitimer(which, seconds[, interval])

   Sets given interval timer (one of :const:`signal.ITIMER_REAL`,
   :const:`signal.ITIMER_VIRTUAL` or :const:`signal.ITIMER_PROF`) specified
   by *which* to fire after *seconds* (float is accepted, different from
   :func:`alarm`) and after that every *interval* seconds. The interval
   timer specified by *which* can be cleared by setting seconds to zero.

   When an interval timer fires, a signal is sent to the process.
   The signal sent is dependent on the timer being used;
   :const:`signal.ITIMER_REAL` will deliver :const:`SIGALRM`,
   :const:`signal.ITIMER_VIRTUAL` sends :const:`SIGVTALRM`,
   and :const:`signal.ITIMER_PROF` will deliver :const:`SIGPROF`.

   The old values are returned as a tuple: (delay, interval).

   Attempting to pass an invalid interval timer will cause an
   :exc:`ItimerError`.  Availability: Unix.


.. function:: getitimer(which)

   Returns current value of a given interval timer specified by *which*.
   Availability: Unix.


.. function:: set_wakeup_fd(fd)

   Set the wakeup file descriptor to *fd*.  When a signal is received, the
   signal number is written as a single byte into the fd.  This can be used by
   a library to wakeup a poll or select call, allowing the signal to be fully
   processed.

   The old wakeup fd is returned.  *fd* must be non-blocking.  It is up to the
   library to remove any bytes before calling poll or select again.

   Use for example ``struct.unpack('%uB' % len(data), data)`` to decode the
   signal numbers list.

   When threads are enabled, this function can only be called from the main thread;
   attempting to call it from other threads will cause a :exc:`ValueError`
   exception to be raised.


.. function:: siginterrupt(signalnum, flag)

   Change system call restart behaviour: if *flag* is :const:`False`, system
   calls will be restarted when interrupted by signal *signalnum*, otherwise
   system calls will be interrupted.  Returns nothing.  Availability: Unix (see
   the man page :manpage:`siginterrupt(3)` for further information).

   Note that installing a signal handler with :func:`signal` will reset the
   restart behaviour to interruptible by implicitly calling
   :c:func:`siginterrupt` with a true *flag* value for the given signal.


.. function:: signal(signalnum, handler)

   Set the handler for signal *signalnum* to the function *handler*.  *handler* can
   be a callable Python object taking two arguments (see below), or one of the
   special values :const:`signal.SIG_IGN` or :const:`signal.SIG_DFL`.  The previous
   signal handler will be returned (see the description of :func:`getsignal`
   above).  (See the Unix man page :manpage:`signal(2)`.)

   When threads are enabled, this function can only be called from the main thread;
   attempting to call it from other threads will cause a :exc:`ValueError`
   exception to be raised.

   The *handler* is called with two arguments: the signal number and the current
   stack frame (``None`` or a frame object; for a description of frame objects,
   see the :ref:`description in the type hierarchy <frame-objects>` or see the
   attribute descriptions in the :mod:`inspect` module).

   On Windows, :func:`signal` can only be called with :const:`SIGABRT`,
   :const:`SIGFPE`, :const:`SIGILL`, :const:`SIGINT`, :const:`SIGSEGV`, or
   :const:`SIGTERM`. A :exc:`ValueError` will be raised in any other case.


.. function:: sigpending()

   Examine the set of signals that are pending for delivery to the calling
   thread (i.e., the signals which have been raised while blocked).  Return the
   set of the pending signals.

   Availability: Unix (see the man page :manpage:`sigpending(2)` for further
   information).

   See also :func:`pause`, :func:`pthread_sigmask` and :func:`sigwait`.

   .. versionadded:: 3.3


.. function:: sigwait(sigset)

   Suspend execution of the calling thread until the delivery of one of the
   signals specified in the signal set *sigset*.  The function accepts the signal
   (removes it from the pending list of signals), and returns the signal number.

   Availability: Unix (see the man page :manpage:`sigwait(3)` for further
   information).

   See also :func:`pause`, :func:`pthread_sigmask`, :func:`sigpending`,
   :func:`sigwaitinfo` and :func:`sigtimedwait`.

   .. versionadded:: 3.3


.. function:: sigwaitinfo(sigset)

   Suspend execution of the calling thread until the delivery of one of the
   signals specified in the signal set *sigset*.  The function accepts the
   signal and removes it from the pending list of signals. If one of the
   signals in *sigset* is already pending for the calling thread, the function
   will return immediately with information about that signal. The signal
   handler is not called for the delivered signal. The function raises an
   :exc:`InterruptedError` if it is interrupted by a signal that is not in
   *sigset*.

   The return value is an object representing the data contained in the
   :c:type:`siginfo_t` structure, namely: :attr:`si_signo`, :attr:`si_code`,
   :attr:`si_errno`, :attr:`si_pid`, :attr:`si_uid`, :attr:`si_status`,
   :attr:`si_band`.

   Availability: Unix (see the man page :manpage:`sigwaitinfo(2)` for further
   information).

   See also :func:`pause`, :func:`sigwait` and :func:`sigtimedwait`.

   .. versionadded:: 3.3


.. function:: sigtimedwait(sigset, timeout)

   Like :func:`sigwaitinfo`, but takes an additional *timeout* argument
   specifying a timeout. If *timeout* is specified as :const:`0`, a poll is
   performed. Returns :const:`None` if a timeout occurs.

   Availability: Unix (see the man page :manpage:`sigtimedwait(2)` for further
   information).

   See also :func:`pause`, :func:`sigwait` and :func:`sigwaitinfo`.

   .. versionadded:: 3.3


.. _signal-example:

Example
-------

Here is a minimal example program. It uses the :func:`alarm` function to limit
the time spent waiting to open a file; this is useful if the file is for a
serial device that may not be turned on, which would normally cause the
:func:`os.open` to hang indefinitely.  The solution is to set a 5-second alarm
before opening the file; if the operation takes too long, the alarm signal will
be sent, and the handler raises an exception. ::

   import signal, os

   def handler(signum, frame):
       print('Signal handler called with signal', signum)
       raise OSError("Couldn't open device!")

   # Set the signal handler and a 5-second alarm
   signal.signal(signal.SIGALRM, handler)
   signal.alarm(5)

   # This open() may hang indefinitely
   fd = os.open('/dev/ttyS0', os.O_RDWR)

   signal.alarm(0)          # Disable the alarm

