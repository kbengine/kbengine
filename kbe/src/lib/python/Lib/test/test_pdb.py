# A test suite for pdb; not very comprehensive at the moment.

import doctest
import os
import pdb
import sys
import types
import unittest
import subprocess
import textwrap

from contextlib import ExitStack
from io import StringIO
from test import support
# This little helper class is essential for testing pdb under doctest.
from test.test_doctest import _FakeInput
from unittest.mock import patch


class PdbTestInput(object):
    """Context manager that makes testing Pdb in doctests easier."""

    def __init__(self, input):
        self.input = input

    def __enter__(self):
        self.real_stdin = sys.stdin
        sys.stdin = _FakeInput(self.input)
        self.orig_trace = sys.gettrace() if hasattr(sys, 'gettrace') else None

    def __exit__(self, *exc):
        sys.stdin = self.real_stdin
        if self.orig_trace:
            sys.settrace(self.orig_trace)


def test_pdb_displayhook():
    """This tests the custom displayhook for pdb.

    >>> def test_function(foo, bar):
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     pass

    >>> with PdbTestInput([
    ...     'foo',
    ...     'bar',
    ...     'for i in range(5): print(i)',
    ...     'continue',
    ... ]):
    ...     test_function(1, None)
    > <doctest test.test_pdb.test_pdb_displayhook[0]>(3)test_function()
    -> pass
    (Pdb) foo
    1
    (Pdb) bar
    (Pdb) for i in range(5): print(i)
    0
    1
    2
    3
    4
    (Pdb) continue
    """


def test_pdb_basic_commands():
    """Test the basic commands of pdb.

    >>> def test_function_2(foo, bar='default'):
    ...     print(foo)
    ...     for i in range(5):
    ...         print(i)
    ...     print(bar)
    ...     for i in range(10):
    ...         never_executed
    ...     print('after for')
    ...     print('...')
    ...     return foo.upper()

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     ret = test_function_2('baz')
    ...     print(ret)

    >>> with PdbTestInput([  # doctest: +ELLIPSIS, +NORMALIZE_WHITESPACE
    ...     'step',       # entering the function call
    ...     'args',       # display function args
    ...     'list',       # list function source
    ...     'bt',         # display backtrace
    ...     'up',         # step up to test_function()
    ...     'down',       # step down to test_function_2() again
    ...     'next',       # stepping to print(foo)
    ...     'next',       # stepping to the for loop
    ...     'step',       # stepping into the for loop
    ...     'until',      # continuing until out of the for loop
    ...     'next',       # executing the print(bar)
    ...     'jump 8',     # jump over second for loop
    ...     'return',     # return out of function
    ...     'retval',     # display return value
    ...     'continue',
    ... ]):
    ...    test_function()
    > <doctest test.test_pdb.test_pdb_basic_commands[1]>(3)test_function()
    -> ret = test_function_2('baz')
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(1)test_function_2()
    -> def test_function_2(foo, bar='default'):
    (Pdb) args
    foo = 'baz'
    bar = 'default'
    (Pdb) list
      1  ->     def test_function_2(foo, bar='default'):
      2             print(foo)
      3             for i in range(5):
      4                 print(i)
      5             print(bar)
      6             for i in range(10):
      7                 never_executed
      8             print('after for')
      9             print('...')
     10             return foo.upper()
    [EOF]
    (Pdb) bt
    ...
      <doctest test.test_pdb.test_pdb_basic_commands[2]>(18)<module>()
    -> test_function()
      <doctest test.test_pdb.test_pdb_basic_commands[1]>(3)test_function()
    -> ret = test_function_2('baz')
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(1)test_function_2()
    -> def test_function_2(foo, bar='default'):
    (Pdb) up
    > <doctest test.test_pdb.test_pdb_basic_commands[1]>(3)test_function()
    -> ret = test_function_2('baz')
    (Pdb) down
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(1)test_function_2()
    -> def test_function_2(foo, bar='default'):
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(2)test_function_2()
    -> print(foo)
    (Pdb) next
    baz
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(3)test_function_2()
    -> for i in range(5):
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(4)test_function_2()
    -> print(i)
    (Pdb) until
    0
    1
    2
    3
    4
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(5)test_function_2()
    -> print(bar)
    (Pdb) next
    default
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(6)test_function_2()
    -> for i in range(10):
    (Pdb) jump 8
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(8)test_function_2()
    -> print('after for')
    (Pdb) return
    after for
    ...
    --Return--
    > <doctest test.test_pdb.test_pdb_basic_commands[0]>(10)test_function_2()->'BAZ'
    -> return foo.upper()
    (Pdb) retval
    'BAZ'
    (Pdb) continue
    BAZ
    """


def test_pdb_breakpoint_commands():
    """Test basic commands related to breakpoints.

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     print(1)
    ...     print(2)
    ...     print(3)
    ...     print(4)

    First, need to clear bdb state that might be left over from previous tests.
    Otherwise, the new breakpoints might get assigned different numbers.

    >>> from bdb import Breakpoint
    >>> Breakpoint.next = 1
    >>> Breakpoint.bplist = {}
    >>> Breakpoint.bpbynumber = [None]

    Now test the breakpoint commands.  NORMALIZE_WHITESPACE is needed because
    the breakpoint list outputs a tab for the "stop only" and "ignore next"
    lines, which we don't want to put in here.

    >>> with PdbTestInput([  # doctest: +NORMALIZE_WHITESPACE
    ...     'break 3',
    ...     'disable 1',
    ...     'ignore 1 10',
    ...     'condition 1 1 < 2',
    ...     'break 4',
    ...     'break 4',
    ...     'break',
    ...     'clear 3',
    ...     'break',
    ...     'condition 1',
    ...     'enable 1',
    ...     'clear 1',
    ...     'commands 2',
    ...     'p "42"',
    ...     'print("42", 7*6)',     # Issue 18764 (not about breakpoints)
    ...     'end',
    ...     'continue',  # will stop at breakpoint 2 (line 4)
    ...     'clear',     # clear all!
    ...     'y',
    ...     'tbreak 5',
    ...     'continue',  # will stop at temporary breakpoint
    ...     'break',     # make sure breakpoint is gone
    ...     'continue',
    ... ]):
    ...    test_function()
    > <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>(3)test_function()
    -> print(1)
    (Pdb) break 3
    Breakpoint 1 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
    (Pdb) disable 1
    Disabled breakpoint 1 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
    (Pdb) ignore 1 10
    Will ignore next 10 crossings of breakpoint 1.
    (Pdb) condition 1 1 < 2
    New condition set for breakpoint 1.
    (Pdb) break 4
    Breakpoint 2 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) break 4
    Breakpoint 3 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) break
    Num Type         Disp Enb   Where
    1   breakpoint   keep no    at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
            stop only if 1 < 2
            ignore next 10 hits
    2   breakpoint   keep yes   at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    3   breakpoint   keep yes   at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) clear 3
    Deleted breakpoint 3 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) break
    Num Type         Disp Enb   Where
    1   breakpoint   keep no    at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
            stop only if 1 < 2
            ignore next 10 hits
    2   breakpoint   keep yes   at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) condition 1
    Breakpoint 1 is now unconditional.
    (Pdb) enable 1
    Enabled breakpoint 1 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
    (Pdb) clear 1
    Deleted breakpoint 1 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:3
    (Pdb) commands 2
    (com) p "42"
    (com) print("42", 7*6)
    (com) end
    (Pdb) continue
    1
    '42'
    42 42
    > <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>(4)test_function()
    -> print(2)
    (Pdb) clear
    Clear all breaks? y
    Deleted breakpoint 2 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:4
    (Pdb) tbreak 5
    Breakpoint 4 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:5
    (Pdb) continue
    2
    Deleted breakpoint 4 at <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>:5
    > <doctest test.test_pdb.test_pdb_breakpoint_commands[0]>(5)test_function()
    -> print(3)
    (Pdb) break
    (Pdb) continue
    3
    4
    """


def do_nothing():
    pass

def do_something():
    print(42)

def test_list_commands():
    """Test the list and source commands of pdb.

    >>> def test_function_2(foo):
    ...     import test.test_pdb
    ...     test.test_pdb.do_nothing()
    ...     'some...'
    ...     'more...'
    ...     'code...'
    ...     'to...'
    ...     'make...'
    ...     'a...'
    ...     'long...'
    ...     'listing...'
    ...     'useful...'
    ...     '...'
    ...     '...'
    ...     return foo

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     ret = test_function_2('baz')

    >>> with PdbTestInput([  # doctest: +ELLIPSIS, +NORMALIZE_WHITESPACE
    ...     'list',      # list first function
    ...     'step',      # step into second function
    ...     'list',      # list second function
    ...     'list',      # continue listing to EOF
    ...     'list 1,3',  # list specific lines
    ...     'list x',    # invalid argument
    ...     'next',      # step to import
    ...     'next',      # step over import
    ...     'step',      # step into do_nothing
    ...     'longlist',  # list all lines
    ...     'source do_something',  # list all lines of function
    ...     'source fooxxx',        # something that doesn't exit
    ...     'continue',
    ... ]):
    ...    test_function()
    > <doctest test.test_pdb.test_list_commands[1]>(3)test_function()
    -> ret = test_function_2('baz')
    (Pdb) list
      1         def test_function():
      2             import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
      3  ->         ret = test_function_2('baz')
    [EOF]
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_list_commands[0]>(1)test_function_2()
    -> def test_function_2(foo):
    (Pdb) list
      1  ->     def test_function_2(foo):
      2             import test.test_pdb
      3             test.test_pdb.do_nothing()
      4             'some...'
      5             'more...'
      6             'code...'
      7             'to...'
      8             'make...'
      9             'a...'
     10             'long...'
     11             'listing...'
    (Pdb) list
     12             'useful...'
     13             '...'
     14             '...'
     15             return foo
    [EOF]
    (Pdb) list 1,3
      1  ->     def test_function_2(foo):
      2             import test.test_pdb
      3             test.test_pdb.do_nothing()
    (Pdb) list x
    *** ...
    (Pdb) next
    > <doctest test.test_pdb.test_list_commands[0]>(2)test_function_2()
    -> import test.test_pdb
    (Pdb) next
    > <doctest test.test_pdb.test_list_commands[0]>(3)test_function_2()
    -> test.test_pdb.do_nothing()
    (Pdb) step
    --Call--
    > ...test_pdb.py(...)do_nothing()
    -> def do_nothing():
    (Pdb) longlist
    ...  ->     def do_nothing():
    ...             pass
    (Pdb) source do_something
    ...         def do_something():
    ...             print(42)
    (Pdb) source fooxxx
    *** ...
    (Pdb) continue
    """


def test_post_mortem():
    """Test post mortem traceback debugging.

    >>> def test_function_2():
    ...     try:
    ...         1/0
    ...     finally:
    ...         print('Exception!')

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     test_function_2()
    ...     print('Not reached.')

    >>> with PdbTestInput([  # doctest: +ELLIPSIS, +NORMALIZE_WHITESPACE
    ...     'next',      # step over exception-raising call
    ...     'bt',        # get a backtrace
    ...     'list',      # list code of test_function()
    ...     'down',      # step into test_function_2()
    ...     'list',      # list code of test_function_2()
    ...     'continue',
    ... ]):
    ...    try:
    ...        test_function()
    ...    except ZeroDivisionError:
    ...        print('Correctly reraised.')
    > <doctest test.test_pdb.test_post_mortem[1]>(3)test_function()
    -> test_function_2()
    (Pdb) next
    Exception!
    ZeroDivisionError: division by zero
    > <doctest test.test_pdb.test_post_mortem[1]>(3)test_function()
    -> test_function_2()
    (Pdb) bt
    ...
      <doctest test.test_pdb.test_post_mortem[2]>(10)<module>()
    -> test_function()
    > <doctest test.test_pdb.test_post_mortem[1]>(3)test_function()
    -> test_function_2()
      <doctest test.test_pdb.test_post_mortem[0]>(3)test_function_2()
    -> 1/0
    (Pdb) list
      1         def test_function():
      2             import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
      3  ->         test_function_2()
      4             print('Not reached.')
    [EOF]
    (Pdb) down
    > <doctest test.test_pdb.test_post_mortem[0]>(3)test_function_2()
    -> 1/0
    (Pdb) list
      1         def test_function_2():
      2             try:
      3  >>             1/0
      4             finally:
      5  ->             print('Exception!')
    [EOF]
    (Pdb) continue
    Correctly reraised.
    """


def test_pdb_skip_modules():
    """This illustrates the simple case of module skipping.

    >>> def skip_module():
    ...     import string
    ...     import pdb; pdb.Pdb(skip=['stri*'], nosigint=True, readrc=False).set_trace()
    ...     string.capwords('FOO')

    >>> with PdbTestInput([
    ...     'step',
    ...     'continue',
    ... ]):
    ...     skip_module()
    > <doctest test.test_pdb.test_pdb_skip_modules[0]>(4)skip_module()
    -> string.capwords('FOO')
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_skip_modules[0]>(4)skip_module()->None
    -> string.capwords('FOO')
    (Pdb) continue
    """


# Module for testing skipping of module that makes a callback
mod = types.ModuleType('module_to_skip')
exec('def foo_pony(callback): x = 1; callback(); return None', mod.__dict__)


def test_pdb_skip_modules_with_callback():
    """This illustrates skipping of modules that call into other code.

    >>> def skip_module():
    ...     def callback():
    ...         return None
    ...     import pdb; pdb.Pdb(skip=['module_to_skip*'], nosigint=True, readrc=False).set_trace()
    ...     mod.foo_pony(callback)

    >>> with PdbTestInput([
    ...     'step',
    ...     'step',
    ...     'step',
    ...     'step',
    ...     'step',
    ...     'continue',
    ... ]):
    ...     skip_module()
    ...     pass  # provides something to "step" to
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[0]>(5)skip_module()
    -> mod.foo_pony(callback)
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[0]>(2)callback()
    -> def callback():
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[0]>(3)callback()
    -> return None
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[0]>(3)callback()->None
    -> return None
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[0]>(5)skip_module()->None
    -> mod.foo_pony(callback)
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_skip_modules_with_callback[1]>(10)<module>()
    -> pass  # provides something to "step" to
    (Pdb) continue
    """


def test_pdb_continue_in_bottomframe():
    """Test that "continue" and "next" work properly in bottom frame (issue #5294).

    >>> def test_function():
    ...     import pdb, sys; inst = pdb.Pdb(nosigint=True, readrc=False)
    ...     inst.set_trace()
    ...     inst.botframe = sys._getframe()  # hackery to get the right botframe
    ...     print(1)
    ...     print(2)
    ...     print(3)
    ...     print(4)

    >>> with PdbTestInput([  # doctest: +ELLIPSIS
    ...     'next',
    ...     'break 7',
    ...     'continue',
    ...     'next',
    ...     'continue',
    ...     'continue',
    ... ]):
    ...    test_function()
    > <doctest test.test_pdb.test_pdb_continue_in_bottomframe[0]>(4)test_function()
    -> inst.botframe = sys._getframe()  # hackery to get the right botframe
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_continue_in_bottomframe[0]>(5)test_function()
    -> print(1)
    (Pdb) break 7
    Breakpoint ... at <doctest test.test_pdb.test_pdb_continue_in_bottomframe[0]>:7
    (Pdb) continue
    1
    2
    > <doctest test.test_pdb.test_pdb_continue_in_bottomframe[0]>(7)test_function()
    -> print(3)
    (Pdb) next
    3
    > <doctest test.test_pdb.test_pdb_continue_in_bottomframe[0]>(8)test_function()
    -> print(4)
    (Pdb) continue
    4
    """


def pdb_invoke(method, arg):
    """Run pdb.method(arg)."""
    getattr(pdb.Pdb(nosigint=True, readrc=False), method)(arg)


def test_pdb_run_with_incorrect_argument():
    """Testing run and runeval with incorrect first argument.

    >>> pti = PdbTestInput(['continue',])
    >>> with pti:
    ...     pdb_invoke('run', lambda x: x)
    Traceback (most recent call last):
    TypeError: exec() arg 1 must be a string, bytes or code object

    >>> with pti:
    ...     pdb_invoke('runeval', lambda x: x)
    Traceback (most recent call last):
    TypeError: eval() arg 1 must be a string, bytes or code object
    """


def test_pdb_run_with_code_object():
    """Testing run and runeval with code object as a first argument.

    >>> with PdbTestInput(['step','x', 'continue']):  # doctest: +ELLIPSIS
    ...     pdb_invoke('run', compile('x=1', '<string>', 'exec'))
    > <string>(1)<module>()...
    (Pdb) step
    --Return--
    > <string>(1)<module>()->None
    (Pdb) x
    1
    (Pdb) continue

    >>> with PdbTestInput(['x', 'continue']):
    ...     x=0
    ...     pdb_invoke('runeval', compile('x+1', '<string>', 'eval'))
    > <string>(1)<module>()->None
    (Pdb) x
    1
    (Pdb) continue
    """

def test_next_until_return_at_return_event():
    """Test that pdb stops after a next/until/return issued at a return debug event.

    >>> def test_function_2():
    ...     x = 1
    ...     x = 2

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     test_function_2()
    ...     test_function_2()
    ...     test_function_2()
    ...     end = 1

    >>> from bdb import Breakpoint
    >>> Breakpoint.next = 1
    >>> with PdbTestInput(['break test_function_2',
    ...                    'continue',
    ...                    'return',
    ...                    'next',
    ...                    'continue',
    ...                    'return',
    ...                    'until',
    ...                    'continue',
    ...                    'return',
    ...                    'return',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_next_until_return_at_return_event[1]>(3)test_function()
    -> test_function_2()
    (Pdb) break test_function_2
    Breakpoint 1 at <doctest test.test_pdb.test_next_until_return_at_return_event[0]>:1
    (Pdb) continue
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(2)test_function_2()
    -> x = 1
    (Pdb) return
    --Return--
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(3)test_function_2()->None
    -> x = 2
    (Pdb) next
    > <doctest test.test_pdb.test_next_until_return_at_return_event[1]>(4)test_function()
    -> test_function_2()
    (Pdb) continue
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(2)test_function_2()
    -> x = 1
    (Pdb) return
    --Return--
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(3)test_function_2()->None
    -> x = 2
    (Pdb) until
    > <doctest test.test_pdb.test_next_until_return_at_return_event[1]>(5)test_function()
    -> test_function_2()
    (Pdb) continue
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(2)test_function_2()
    -> x = 1
    (Pdb) return
    --Return--
    > <doctest test.test_pdb.test_next_until_return_at_return_event[0]>(3)test_function_2()->None
    -> x = 2
    (Pdb) return
    > <doctest test.test_pdb.test_next_until_return_at_return_event[1]>(6)test_function()
    -> end = 1
    (Pdb) continue
    """

def test_pdb_next_command_for_generator():
    """Testing skip unwindng stack on yield for generators for "next" command

    >>> def test_gen():
    ...     yield 0
    ...     return 1
    ...     yield 2

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     it = test_gen()
    ...     try:
    ...         if next(it) != 0:
    ...             raise AssertionError
    ...         next(it)
    ...     except StopIteration as ex:
    ...         if ex.value != 1:
    ...             raise AssertionError
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'step',
    ...                    'next',
    ...                    'next',
    ...                    'step',
    ...                    'step',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[1]>(3)test_function()
    -> it = test_gen()
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[1]>(4)test_function()
    -> try:
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[1]>(5)test_function()
    -> if next(it) != 0:
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[0]>(1)test_gen()
    -> def test_gen():
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[0]>(2)test_gen()
    -> yield 0
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[0]>(3)test_gen()
    -> return 1
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[0]>(3)test_gen()->1
    -> return 1
    (Pdb) step
    StopIteration: 1
    > <doctest test.test_pdb.test_pdb_next_command_for_generator[1]>(7)test_function()
    -> next(it)
    (Pdb) continue
    finished
    """

def test_pdb_next_command_for_coroutine():
    """Testing skip unwindng stack on yield for coroutines for "next" command

    >>> import asyncio

    >>> async def test_coro():
    ...     await asyncio.sleep(0)
    ...     await asyncio.sleep(0)
    ...     await asyncio.sleep(0)

    >>> async def test_main():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     await test_coro()

    >>> def test_function():
    ...     loop = asyncio.new_event_loop()
    ...     loop.run_until_complete(test_main())
    ...     loop.close()
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'next',
    ...                    'next',
    ...                    'next',
    ...                    'step',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[2]>(3)test_main()
    -> await test_coro()
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[1]>(1)test_coro()
    -> async def test_coro():
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[1]>(2)test_coro()
    -> await asyncio.sleep(0)
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[1]>(3)test_coro()
    -> await asyncio.sleep(0)
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[1]>(4)test_coro()
    -> await asyncio.sleep(0)
    (Pdb) next
    Internal StopIteration
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[2]>(3)test_main()
    -> await test_coro()
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_next_command_for_coroutine[2]>(3)test_main()->None
    -> await test_coro()
    (Pdb) continue
    finished
    """

def test_pdb_next_command_for_asyncgen():
    """Testing skip unwindng stack on yield for coroutines for "next" command

    >>> import asyncio

    >>> async def agen():
    ...     yield 1
    ...     await asyncio.sleep(0)
    ...     yield 2

    >>> async def test_coro():
    ...     async for x in agen():
    ...         print(x)

    >>> async def test_main():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     await test_coro()

    >>> def test_function():
    ...     loop = asyncio.new_event_loop()
    ...     loop.run_until_complete(test_main())
    ...     loop.close()
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'next',
    ...                    'next',
    ...                    'step',
    ...                    'next',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[3]>(3)test_main()
    -> await test_coro()
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[2]>(1)test_coro()
    -> async def test_coro():
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[2]>(2)test_coro()
    -> async for x in agen():
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[2]>(3)test_coro()
    -> print(x)
    (Pdb) next
    1
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[2]>(2)test_coro()
    -> async for x in agen():
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[1]>(2)agen()
    -> yield 1
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_for_asyncgen[1]>(3)agen()
    -> await asyncio.sleep(0)
    (Pdb) continue
    2
    finished
    """

def test_pdb_return_command_for_generator():
    """Testing no unwindng stack on yield for generators
       for "return" command

    >>> def test_gen():
    ...     yield 0
    ...     return 1
    ...     yield 2

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     it = test_gen()
    ...     try:
    ...         if next(it) != 0:
    ...             raise AssertionError
    ...         next(it)
    ...     except StopIteration as ex:
    ...         if ex.value != 1:
    ...             raise AssertionError
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'step',
    ...                    'return',
    ...                    'step',
    ...                    'step',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(3)test_function()
    -> it = test_gen()
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(4)test_function()
    -> try:
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(5)test_function()
    -> if next(it) != 0:
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[0]>(1)test_gen()
    -> def test_gen():
    (Pdb) return
    StopIteration: 1
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(7)test_function()
    -> next(it)
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(8)test_function()
    -> except StopIteration as ex:
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_return_command_for_generator[1]>(9)test_function()
    -> if ex.value != 1:
    (Pdb) continue
    finished
    """

def test_pdb_return_command_for_coroutine():
    """Testing no unwindng stack on yield for coroutines for "return" command

    >>> import asyncio

    >>> async def test_coro():
    ...     await asyncio.sleep(0)
    ...     await asyncio.sleep(0)
    ...     await asyncio.sleep(0)

    >>> async def test_main():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     await test_coro()

    >>> def test_function():
    ...     loop = asyncio.new_event_loop()
    ...     loop.run_until_complete(test_main())
    ...     loop.close()
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'next',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_return_command_for_coroutine[2]>(3)test_main()
    -> await test_coro()
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_return_command_for_coroutine[1]>(1)test_coro()
    -> async def test_coro():
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_return_command_for_coroutine[1]>(2)test_coro()
    -> await asyncio.sleep(0)
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_return_command_for_coroutine[1]>(3)test_coro()
    -> await asyncio.sleep(0)
    (Pdb) continue
    finished
    """

def test_pdb_until_command_for_generator():
    """Testing no unwindng stack on yield for generators
       for "until" command if target breakpoing is not reached

    >>> def test_gen():
    ...     yield 0
    ...     yield 1
    ...     yield 2

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     for i in test_gen():
    ...         print(i)
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'until 4',
    ...                    'step',
    ...                    'step',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_until_command_for_generator[1]>(3)test_function()
    -> for i in test_gen():
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_until_command_for_generator[0]>(1)test_gen()
    -> def test_gen():
    (Pdb) until 4
    0
    1
    > <doctest test.test_pdb.test_pdb_until_command_for_generator[0]>(4)test_gen()
    -> yield 2
    (Pdb) step
    --Return--
    > <doctest test.test_pdb.test_pdb_until_command_for_generator[0]>(4)test_gen()->2
    -> yield 2
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_until_command_for_generator[1]>(4)test_function()
    -> print(i)
    (Pdb) continue
    2
    finished
    """

def test_pdb_until_command_for_coroutine():
    """Testing no unwindng stack for coroutines
       for "until" command if target breakpoing is not reached

    >>> import asyncio

    >>> async def test_coro():
    ...     print(0)
    ...     await asyncio.sleep(0)
    ...     print(1)
    ...     await asyncio.sleep(0)
    ...     print(2)
    ...     await asyncio.sleep(0)
    ...     print(3)

    >>> async def test_main():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     await test_coro()

    >>> def test_function():
    ...     loop = asyncio.new_event_loop()
    ...     loop.run_until_complete(test_main())
    ...     loop.close()
    ...     print("finished")

    >>> with PdbTestInput(['step',
    ...                    'until 8',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_until_command_for_coroutine[2]>(3)test_main()
    -> await test_coro()
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_until_command_for_coroutine[1]>(1)test_coro()
    -> async def test_coro():
    (Pdb) until 8
    0
    1
    2
    > <doctest test.test_pdb.test_pdb_until_command_for_coroutine[1]>(8)test_coro()
    -> print(3)
    (Pdb) continue
    3
    finished
    """

def test_pdb_next_command_in_generator_for_loop():
    """The next command on returning from a generator controlled by a for loop.

    >>> def test_gen():
    ...     yield 0
    ...     return 1

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     for i in test_gen():
    ...         print('value', i)
    ...     x = 123

    >>> with PdbTestInput(['break test_gen',
    ...                    'continue',
    ...                    'next',
    ...                    'next',
    ...                    'next',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[1]>(3)test_function()
    -> for i in test_gen():
    (Pdb) break test_gen
    Breakpoint 6 at <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[0]>:1
    (Pdb) continue
    > <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[0]>(2)test_gen()
    -> yield 0
    (Pdb) next
    value 0
    > <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[0]>(3)test_gen()
    -> return 1
    (Pdb) next
    Internal StopIteration: 1
    > <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[1]>(3)test_function()
    -> for i in test_gen():
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_in_generator_for_loop[1]>(5)test_function()
    -> x = 123
    (Pdb) continue
    """

def test_pdb_next_command_subiterator():
    """The next command in a generator with a subiterator.

    >>> def test_subgenerator():
    ...     yield 0
    ...     return 1

    >>> def test_gen():
    ...     x = yield from test_subgenerator()
    ...     return x

    >>> def test_function():
    ...     import pdb; pdb.Pdb(nosigint=True, readrc=False).set_trace()
    ...     for i in test_gen():
    ...         print('value', i)
    ...     x = 123

    >>> with PdbTestInput(['step',
    ...                    'step',
    ...                    'next',
    ...                    'next',
    ...                    'next',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[2]>(3)test_function()
    -> for i in test_gen():
    (Pdb) step
    --Call--
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[1]>(1)test_gen()
    -> def test_gen():
    (Pdb) step
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[1]>(2)test_gen()
    -> x = yield from test_subgenerator()
    (Pdb) next
    value 0
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[1]>(3)test_gen()
    -> return x
    (Pdb) next
    Internal StopIteration: 1
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[2]>(3)test_function()
    -> for i in test_gen():
    (Pdb) next
    > <doctest test.test_pdb.test_pdb_next_command_subiterator[2]>(5)test_function()
    -> x = 123
    (Pdb) continue
    """

def test_pdb_issue_20766():
    """Test for reference leaks when the SIGINT handler is set.

    >>> def test_function():
    ...     i = 1
    ...     while i <= 2:
    ...         sess = pdb.Pdb()
    ...         sess.set_trace(sys._getframe())
    ...         print('pdb %d: %s' % (i, sess._previous_sigint_handler))
    ...         i += 1

    >>> with PdbTestInput(['continue',
    ...                    'continue']):
    ...     test_function()
    > <doctest test.test_pdb.test_pdb_issue_20766[0]>(6)test_function()
    -> print('pdb %d: %s' % (i, sess._previous_sigint_handler))
    (Pdb) continue
    pdb 1: <built-in function default_int_handler>
    > <doctest test.test_pdb.test_pdb_issue_20766[0]>(5)test_function()
    -> sess.set_trace(sys._getframe())
    (Pdb) continue
    pdb 2: <built-in function default_int_handler>
    """


class PdbTestCase(unittest.TestCase):
    def tearDown(self):
        support.unlink(support.TESTFN)

    def _run_pdb(self, pdb_args, commands):
        self.addCleanup(support.rmtree, '__pycache__')
        cmd = [sys.executable, '-m', 'pdb'] + pdb_args
        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stdin=subprocess.PIPE,
                stderr=subprocess.STDOUT,
        ) as proc:
            stdout, stderr = proc.communicate(str.encode(commands))
        stdout = stdout and bytes.decode(stdout)
        stderr = stderr and bytes.decode(stderr)
        return stdout, stderr

    def run_pdb_script(self, script, commands):
        """Run 'script' lines with pdb and the pdb 'commands'."""
        filename = 'main.py'
        with open(filename, 'w') as f:
            f.write(textwrap.dedent(script))
        self.addCleanup(support.unlink, filename)
        return self._run_pdb([filename], commands)

    def run_pdb_module(self, script, commands):
        """Runs the script code as part of a module"""
        self.module_name = 't_main'
        support.rmtree(self.module_name)
        main_file = self.module_name + '/__main__.py'
        init_file = self.module_name + '/__init__.py'
        os.mkdir(self.module_name)
        with open(init_file, 'w') as f:
            pass
        with open(main_file, 'w') as f:
            f.write(textwrap.dedent(script))
        self.addCleanup(support.rmtree, self.module_name)
        return self._run_pdb(['-m', self.module_name], commands)

    def _assert_find_function(self, file_content, func_name, expected):
        file_content = textwrap.dedent(file_content)

        with open(support.TESTFN, 'w') as f:
            f.write(file_content)

        expected = None if not expected else (
            expected[0], support.TESTFN, expected[1])
        self.assertEqual(
            expected, pdb.find_function(func_name, support.TESTFN))

    def test_find_function_empty_file(self):
        self._assert_find_function('', 'foo', None)

    def test_find_function_found(self):
        self._assert_find_function(
            """\
            def foo():
                pass

            def bar():
                pass

            def quux():
                pass
            """,
            'bar',
            ('bar', 4),
        )

    def test_issue7964(self):
        # open the file as binary so we can force \r\n newline
        with open(support.TESTFN, 'wb') as f:
            f.write(b'print("testing my pdb")\r\n')
        cmd = [sys.executable, '-m', 'pdb', support.TESTFN]
        proc = subprocess.Popen(cmd,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            )
        self.addCleanup(proc.stdout.close)
        stdout, stderr = proc.communicate(b'quit\n')
        self.assertNotIn(b'SyntaxError', stdout,
                         "Got a syntax error running test script under PDB")

    def test_issue13183(self):
        script = """
            from bar import bar

            def foo():
                bar()

            def nope():
                pass

            def foobar():
                foo()
                nope()

            foobar()
        """
        commands = """
            from bar import bar
            break bar
            continue
            step
            step
            quit
        """
        bar = """
            def bar():
                pass
        """
        with open('bar.py', 'w') as f:
            f.write(textwrap.dedent(bar))
        self.addCleanup(support.unlink, 'bar.py')
        stdout, stderr = self.run_pdb_script(script, commands)
        self.assertTrue(
            any('main.py(5)foo()->None' in l for l in stdout.splitlines()),
            'Fail to step into the caller after a return')

    def test_issue13210(self):
        # invoking "continue" on a non-main thread triggered an exception
        # inside signal.signal

        with open(support.TESTFN, 'wb') as f:
            f.write(textwrap.dedent("""
                import threading
                import pdb

                def start_pdb():
                    pdb.Pdb(readrc=False).set_trace()
                    x = 1
                    y = 1

                t = threading.Thread(target=start_pdb)
                t.start()""").encode('ascii'))
        cmd = [sys.executable, '-u', support.TESTFN]
        proc = subprocess.Popen(cmd,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            )
        self.addCleanup(proc.stdout.close)
        stdout, stderr = proc.communicate(b'cont\n')
        self.assertNotIn('Error', stdout.decode(),
                         "Got an error running test script under PDB")

    def test_issue16180(self):
        # A syntax error in the debuggee.
        script = "def f: pass\n"
        commands = ''
        expected = "SyntaxError:"
        stdout, stderr = self.run_pdb_script(script, commands)
        self.assertIn(expected, stdout,
            '\n\nExpected:\n{}\nGot:\n{}\n'
            'Fail to handle a syntax error in the debuggee.'
            .format(expected, stdout))


    def test_readrc_kwarg(self):
        script = textwrap.dedent("""
            import pdb; pdb.Pdb(readrc=False).set_trace()

            print('hello')
        """)

        save_home = os.environ.pop('HOME', None)
        try:
            with support.temp_cwd():
                with open('.pdbrc', 'w') as f:
                    f.write("invalid\n")

                with open('main.py', 'w') as f:
                    f.write(script)

                cmd = [sys.executable, 'main.py']
                proc = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stdin=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                )
                with proc:
                    stdout, stderr = proc.communicate(b'q\n')
                    self.assertNotIn("NameError: name 'invalid' is not defined",
                                  stdout.decode())

        finally:
            if save_home is not None:
                os.environ['HOME'] = save_home

    def test_header(self):
        stdout = StringIO()
        header = 'Nobody expects... blah, blah, blah'
        with ExitStack() as resources:
            resources.enter_context(patch('sys.stdout', stdout))
            resources.enter_context(patch.object(pdb.Pdb, 'set_trace'))
            pdb.set_trace(header=header)
        self.assertEqual(stdout.getvalue(), header + '\n')

    def test_run_module(self):
        script = """print("SUCCESS")"""
        commands = """
            continue
            quit
        """
        stdout, stderr = self.run_pdb_module(script, commands)
        self.assertTrue(any("SUCCESS" in l for l in stdout.splitlines()), stdout)

    def test_module_is_run_as_main(self):
        script = """
            if __name__ == '__main__':
                print("SUCCESS")
        """
        commands = """
            continue
            quit
        """
        stdout, stderr = self.run_pdb_module(script, commands)
        self.assertTrue(any("SUCCESS" in l for l in stdout.splitlines()), stdout)

    def test_breakpoint(self):
        script = """
            if __name__ == '__main__':
                pass
                print("SUCCESS")
                pass
        """
        commands = """
            b 3
            quit
        """
        stdout, stderr = self.run_pdb_module(script, commands)
        self.assertTrue(any("Breakpoint 1 at" in l for l in stdout.splitlines()), stdout)
        self.assertTrue(all("SUCCESS" not in l for l in stdout.splitlines()), stdout)

    def test_run_pdb_with_pdb(self):
        commands = """
            c
            quit
        """
        stdout, stderr = self._run_pdb(["-m", "pdb"], commands)
        self.assertIn(
            pdb._usage,
            stdout.replace('\r', '')  # remove \r for windows
        )

    def test_module_without_a_main(self):
        module_name = 't_main'
        support.rmtree(module_name)
        init_file = module_name + '/__init__.py'
        os.mkdir(module_name)
        with open(init_file, 'w') as f:
            pass
        self.addCleanup(support.rmtree, module_name)
        stdout, stderr = self._run_pdb(['-m', module_name], "")
        self.assertIn("ImportError: No module named t_main.__main__",
                      stdout.splitlines())

    def test_blocks_at_first_code_line(self):
        script = """
                #This is a comment, on line 2

                print("SUCCESS")
        """
        commands = """
            quit
        """
        stdout, stderr = self.run_pdb_module(script, commands)
        self.assertTrue(any("__main__.py(4)<module>()"
                            in l for l in stdout.splitlines()), stdout)

    def test_relative_imports(self):
        self.module_name = 't_main'
        support.rmtree(self.module_name)
        main_file = self.module_name + '/__main__.py'
        init_file = self.module_name + '/__init__.py'
        module_file = self.module_name + '/module.py'
        self.addCleanup(support.rmtree, self.module_name)
        os.mkdir(self.module_name)
        with open(init_file, 'w') as f:
            f.write(textwrap.dedent("""
                top_var = "VAR from top"
            """))
        with open(main_file, 'w') as f:
            f.write(textwrap.dedent("""
                from . import top_var
                from .module import var
                from . import module
                pass # We'll stop here and print the vars
            """))
        with open(module_file, 'w') as f:
            f.write(textwrap.dedent("""
                var = "VAR from module"
                var2 = "second var"
            """))
        commands = """
            b 5
            c
            p top_var
            p var
            p module.var2
            quit
        """
        stdout, _ = self._run_pdb(['-m', self.module_name], commands)
        self.assertTrue(any("VAR from module" in l for l in stdout.splitlines()), stdout)
        self.assertTrue(any("VAR from top" in l for l in stdout.splitlines()))
        self.assertTrue(any("second var" in l for l in stdout.splitlines()))

    def test_relative_imports_on_plain_module(self):
        # Validates running a plain module. See bpo32691
        self.module_name = 't_main'
        support.rmtree(self.module_name)
        main_file = self.module_name + '/runme.py'
        init_file = self.module_name + '/__init__.py'
        module_file = self.module_name + '/module.py'
        self.addCleanup(support.rmtree, self.module_name)
        os.mkdir(self.module_name)
        with open(init_file, 'w') as f:
            f.write(textwrap.dedent("""
                top_var = "VAR from top"
            """))
        with open(main_file, 'w') as f:
            f.write(textwrap.dedent("""
                from . import module
                pass # We'll stop here and print the vars
            """))
        with open(module_file, 'w') as f:
            f.write(textwrap.dedent("""
                var = "VAR from module"
            """))
        commands = """
            b 3
            c
            p module.var
            quit
        """
        stdout, _ = self._run_pdb(['-m', self.module_name + '.runme'], commands)
        self.assertTrue(any("VAR from module" in l for l in stdout.splitlines()), stdout)


def load_tests(*args):
    from test import test_pdb
    suites = [
        unittest.makeSuite(PdbTestCase),
        doctest.DocTestSuite(test_pdb)
    ]
    return unittest.TestSuite(suites)


if __name__ == '__main__':
    unittest.main()
