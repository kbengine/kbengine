# Verify that gdb can pretty-print the various PyObject* types
#
# The code for testing gdb was adapted from similar work in Unladen Swallow's
# Lib/test/test_jit_gdb.py

import os
import re
import pprint
import subprocess
import sys
import sysconfig
import unittest
import locale

# Is this Python configured to support threads?
try:
    import _thread
except ImportError:
    _thread = None

from test import support
from test.support import run_unittest, findfile, python_is_optimized

try:
    gdb_version, _ = subprocess.Popen(["gdb", "--version"],
                                      stdout=subprocess.PIPE).communicate()
except OSError:
    # This is what "no gdb" looks like.  There may, however, be other
    # errors that manifest this way too.
    raise unittest.SkipTest("Couldn't find gdb on the path")
gdb_version_number = re.search(b"^GNU gdb [^\d]*(\d+)\.(\d)", gdb_version)
gdb_major_version = int(gdb_version_number.group(1))
gdb_minor_version = int(gdb_version_number.group(2))
if gdb_major_version < 7:
    raise unittest.SkipTest("gdb versions before 7.0 didn't support python embedding"
                            " Saw:\n" + gdb_version.decode('ascii', 'replace'))

if not sysconfig.is_python_build():
    raise unittest.SkipTest("test_gdb only works on source builds at the moment.")

# Location of custom hooks file in a repository checkout.
checkout_hook_path = os.path.join(os.path.dirname(sys.executable),
                                  'python-gdb.py')

PYTHONHASHSEED = '123'

def run_gdb(*args, **env_vars):
    """Runs gdb in --batch mode with the additional arguments given by *args.

    Returns its (stdout, stderr) decoded from utf-8 using the replace handler.
    """
    if env_vars:
        env = os.environ.copy()
        env.update(env_vars)
    else:
        env = None
    base_cmd = ('gdb', '--batch')
    if (gdb_major_version, gdb_minor_version) >= (7, 4):
        base_cmd += ('-iex', 'add-auto-load-safe-path ' + checkout_hook_path)
    out, err = subprocess.Popen(base_cmd + args,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env,
        ).communicate()
    return out.decode('utf-8', 'replace'), err.decode('utf-8', 'replace')

# Verify that "gdb" was built with the embedded python support enabled:
gdbpy_version, _ = run_gdb("--eval-command=python import sys; print(sys.version_info)")
if not gdbpy_version:
    raise unittest.SkipTest("gdb not built with embedded python support")

# Verify that "gdb" can load our custom hooks, as OS security settings may
# disallow this without a customised .gdbinit.
cmd = ['--args', sys.executable]
_, gdbpy_errors = run_gdb('--args', sys.executable)
if "auto-loading has been declined" in gdbpy_errors:
    msg = "gdb security settings prevent use of custom hooks: "
    raise unittest.SkipTest(msg + gdbpy_errors.rstrip())

def gdb_has_frame_select():
    # Does this build of gdb have gdb.Frame.select ?
    stdout, _ = run_gdb("--eval-command=python print(dir(gdb.Frame))")
    m = re.match(r'.*\[(.*)\].*', stdout)
    if not m:
        raise unittest.SkipTest("Unable to parse output from gdb.Frame.select test")
    gdb_frame_dir = m.group(1).split(', ')
    return "'select'" in gdb_frame_dir

HAS_PYUP_PYDOWN = gdb_has_frame_select()

BREAKPOINT_FN='builtin_id'

class DebuggerTests(unittest.TestCase):

    """Test that the debugger can debug Python."""

    def get_stack_trace(self, source=None, script=None,
                        breakpoint=BREAKPOINT_FN,
                        cmds_after_breakpoint=None,
                        import_site=False):
        '''
        Run 'python -c SOURCE' under gdb with a breakpoint.

        Support injecting commands after the breakpoint is reached

        Returns the stdout from gdb

        cmds_after_breakpoint: if provided, a list of strings: gdb commands
        '''
        # We use "set breakpoint pending yes" to avoid blocking with a:
        #   Function "foo" not defined.
        #   Make breakpoint pending on future shared library load? (y or [n])
        # error, which typically happens python is dynamically linked (the
        # breakpoints of interest are to be found in the shared library)
        # When this happens, we still get:
        #   Function "textiowrapper_write" not defined.
        # emitted to stderr each time, alas.

        # Initially I had "--eval-command=continue" here, but removed it to
        # avoid repeated print breakpoints when traversing hierarchical data
        # structures

        # Generate a list of commands in gdb's language:
        commands = ['set breakpoint pending yes',
                    'break %s' % breakpoint,
                    'run']
        if cmds_after_breakpoint:
            commands += cmds_after_breakpoint
        else:
            commands += ['backtrace']

        # print commands

        # Use "commands" to generate the arguments with which to invoke "gdb":
        args = ["gdb", "--batch"]
        args += ['--eval-command=%s' % cmd for cmd in commands]
        args += ["--args",
                 sys.executable]

        if not import_site:
            # -S suppresses the default 'import site'
            args += ["-S"]

        if source:
            args += ["-c", source]
        elif script:
            args += [script]

        # print args
        # print (' '.join(args))

        # Use "args" to invoke gdb, capturing stdout, stderr:
        out, err = run_gdb(*args, PYTHONHASHSEED=PYTHONHASHSEED)

        errlines = err.splitlines()
        unexpected_errlines = []

        # Ignore some benign messages on stderr.
        ignore_patterns = (
            'Function "%s" not defined.' % breakpoint,
            "warning: no loadable sections found in added symbol-file"
            " system-supplied DSO",
            "warning: Unable to find libthread_db matching"
            " inferior's thread library, thread debugging will"
            " not be available.",
            "warning: Cannot initialize thread debugging"
            " library: Debugger service failed",
            'warning: Could not load shared library symbols for '
            'linux-vdso.so',
            'warning: Could not load shared library symbols for '
            'linux-gate.so',
            'Do you need "set solib-search-path" or '
            '"set sysroot"?',
            'warning: Source file is more recent than executable.',
            # Issue #19753: missing symbols on System Z
            'Missing separate debuginfo for ',
            'Try: zypper install -C ',
            )
        for line in errlines:
            if not line.startswith(ignore_patterns):
                unexpected_errlines.append(line)

        # Ensure no unexpected error messages:
        self.assertEqual(unexpected_errlines, [])
        return out

    def get_gdb_repr(self, source,
                     cmds_after_breakpoint=None,
                     import_site=False):
        # Given an input python source representation of data,
        # run "python -c'id(DATA)'" under gdb with a breakpoint on
        # builtin_id and scrape out gdb's representation of the "op"
        # parameter, and verify that the gdb displays the same string
        #
        # Verify that the gdb displays the expected string
        #
        # For a nested structure, the first time we hit the breakpoint will
        # give us the top-level structure

        # NOTE: avoid decoding too much of the traceback as some
        # undecodable characters may lurk there in optimized mode
        # (issue #19743).
        cmds_after_breakpoint = cmds_after_breakpoint or ["backtrace 1"]
        gdb_output = self.get_stack_trace(source, breakpoint=BREAKPOINT_FN,
                                          cmds_after_breakpoint=cmds_after_breakpoint,
                                          import_site=import_site)
        # gdb can insert additional '\n' and space characters in various places
        # in its output, depending on the width of the terminal it's connected
        # to (using its "wrap_here" function)
        m = re.match('.*#0\s+builtin_id\s+\(self\=.*,\s+v=\s*(.*?)\)\s+at\s+\S*Python/bltinmodule.c.*',
                     gdb_output, re.DOTALL)
        if not m:
            self.fail('Unexpected gdb output: %r\n%s' % (gdb_output, gdb_output))
        return m.group(1), gdb_output

    def assertEndsWith(self, actual, exp_end):
        '''Ensure that the given "actual" string ends with "exp_end"'''
        self.assertTrue(actual.endswith(exp_end),
                        msg='%r did not end with %r' % (actual, exp_end))

    def assertMultilineMatches(self, actual, pattern):
        m = re.match(pattern, actual, re.DOTALL)
        if not m:
            self.fail(msg='%r did not match %r' % (actual, pattern))

    def get_sample_script(self):
        return findfile('gdb_sample.py')

class PrettyPrintTests(DebuggerTests):
    def test_getting_backtrace(self):
        gdb_output = self.get_stack_trace('id(42)')
        self.assertTrue(BREAKPOINT_FN in gdb_output)

    def assertGdbRepr(self, val, exp_repr=None):
        # Ensure that gdb's rendering of the value in a debugged process
        # matches repr(value) in this process:
        gdb_repr, gdb_output = self.get_gdb_repr('id(' + ascii(val) + ')')
        if not exp_repr:
            exp_repr = repr(val)
        self.assertEqual(gdb_repr, exp_repr,
                         ('%r did not equal expected %r; full output was:\n%s'
                          % (gdb_repr, exp_repr, gdb_output)))

    def test_int(self):
        'Verify the pretty-printing of various int values'
        self.assertGdbRepr(42)
        self.assertGdbRepr(0)
        self.assertGdbRepr(-7)
        self.assertGdbRepr(1000000000000)
        self.assertGdbRepr(-1000000000000000)

    def test_singletons(self):
        'Verify the pretty-printing of True, False and None'
        self.assertGdbRepr(True)
        self.assertGdbRepr(False)
        self.assertGdbRepr(None)

    def test_dicts(self):
        'Verify the pretty-printing of dictionaries'
        self.assertGdbRepr({})
        self.assertGdbRepr({'foo': 'bar'}, "{'foo': 'bar'}")
        self.assertGdbRepr({'foo': 'bar', 'douglas': 42}, "{'douglas': 42, 'foo': 'bar'}")

    def test_lists(self):
        'Verify the pretty-printing of lists'
        self.assertGdbRepr([])
        self.assertGdbRepr(list(range(5)))

    def test_bytes(self):
        'Verify the pretty-printing of bytes'
        self.assertGdbRepr(b'')
        self.assertGdbRepr(b'And now for something hopefully the same')
        self.assertGdbRepr(b'string with embedded NUL here \0 and then some more text')
        self.assertGdbRepr(b'this is a tab:\t'
                           b' this is a slash-N:\n'
                           b' this is a slash-R:\r'
                           )

        self.assertGdbRepr(b'this is byte 255:\xff and byte 128:\x80')

        self.assertGdbRepr(bytes([b for b in range(255)]))

    def test_strings(self):
        'Verify the pretty-printing of unicode strings'
        encoding = locale.getpreferredencoding()
        def check_repr(text):
            try:
                text.encode(encoding)
                printable = True
            except UnicodeEncodeError:
                self.assertGdbRepr(text, ascii(text))
            else:
                self.assertGdbRepr(text)

        self.assertGdbRepr('')
        self.assertGdbRepr('And now for something hopefully the same')
        self.assertGdbRepr('string with embedded NUL here \0 and then some more text')

        # Test printing a single character:
        #    U+2620 SKULL AND CROSSBONES
        check_repr('\u2620')

        # Test printing a Japanese unicode string
        # (I believe this reads "mojibake", using 3 characters from the CJK
        # Unified Ideographs area, followed by U+3051 HIRAGANA LETTER KE)
        check_repr('\u6587\u5b57\u5316\u3051')

        # Test a character outside the BMP:
        #    U+1D121 MUSICAL SYMBOL C CLEF
        # This is:
        # UTF-8: 0xF0 0x9D 0x84 0xA1
        # UTF-16: 0xD834 0xDD21
        check_repr(chr(0x1D121))

    def test_tuples(self):
        'Verify the pretty-printing of tuples'
        self.assertGdbRepr(tuple(), '()')
        self.assertGdbRepr((1,), '(1,)')
        self.assertGdbRepr(('foo', 'bar', 'baz'))

    def test_sets(self):
        'Verify the pretty-printing of sets'
        if (gdb_major_version, gdb_minor_version) < (7, 3):
            self.skipTest("pretty-printing of sets needs gdb 7.3 or later")
        self.assertGdbRepr(set(), 'set()')
        self.assertGdbRepr(set(['a', 'b']), "{'a', 'b'}")
        self.assertGdbRepr(set([4, 5, 6]), "{4, 5, 6}")

        # Ensure that we handle sets containing the "dummy" key value,
        # which happens on deletion:
        gdb_repr, gdb_output = self.get_gdb_repr('''s = set(['a','b'])
s.remove('a')
id(s)''')
        self.assertEqual(gdb_repr, "{'b'}")

    def test_frozensets(self):
        'Verify the pretty-printing of frozensets'
        if (gdb_major_version, gdb_minor_version) < (7, 3):
            self.skipTest("pretty-printing of frozensets needs gdb 7.3 or later")
        self.assertGdbRepr(frozenset(), 'frozenset()')
        self.assertGdbRepr(frozenset(['a', 'b']), "frozenset({'a', 'b'})")
        self.assertGdbRepr(frozenset([4, 5, 6]), "frozenset({4, 5, 6})")

    def test_exceptions(self):
        # Test a RuntimeError
        gdb_repr, gdb_output = self.get_gdb_repr('''
try:
    raise RuntimeError("I am an error")
except RuntimeError as e:
    id(e)
''')
        self.assertEqual(gdb_repr,
                         "RuntimeError('I am an error',)")


        # Test division by zero:
        gdb_repr, gdb_output = self.get_gdb_repr('''
try:
    a = 1 / 0
except ZeroDivisionError as e:
    id(e)
''')
        self.assertEqual(gdb_repr,
                         "ZeroDivisionError('division by zero',)")

    def test_modern_class(self):
        'Verify the pretty-printing of new-style class instances'
        gdb_repr, gdb_output = self.get_gdb_repr('''
class Foo:
    pass
foo = Foo()
foo.an_int = 42
id(foo)''')
        m = re.match(r'<Foo\(an_int=42\) at remote 0x-?[0-9a-f]+>', gdb_repr)
        self.assertTrue(m,
                        msg='Unexpected new-style class rendering %r' % gdb_repr)

    def test_subclassing_list(self):
        'Verify the pretty-printing of an instance of a list subclass'
        gdb_repr, gdb_output = self.get_gdb_repr('''
class Foo(list):
    pass
foo = Foo()
foo += [1, 2, 3]
foo.an_int = 42
id(foo)''')
        m = re.match(r'<Foo\(an_int=42\) at remote 0x-?[0-9a-f]+>', gdb_repr)

        self.assertTrue(m,
                        msg='Unexpected new-style class rendering %r' % gdb_repr)

    def test_subclassing_tuple(self):
        'Verify the pretty-printing of an instance of a tuple subclass'
        # This should exercise the negative tp_dictoffset code in the
        # new-style class support
        gdb_repr, gdb_output = self.get_gdb_repr('''
class Foo(tuple):
    pass
foo = Foo((1, 2, 3))
foo.an_int = 42
id(foo)''')
        m = re.match(r'<Foo\(an_int=42\) at remote 0x-?[0-9a-f]+>', gdb_repr)

        self.assertTrue(m,
                        msg='Unexpected new-style class rendering %r' % gdb_repr)

    def assertSane(self, source, corruption, exprepr=None):
        '''Run Python under gdb, corrupting variables in the inferior process
        immediately before taking a backtrace.

        Verify that the variable's representation is the expected failsafe
        representation'''
        if corruption:
            cmds_after_breakpoint=[corruption, 'backtrace']
        else:
            cmds_after_breakpoint=['backtrace']

        gdb_repr, gdb_output = \
            self.get_gdb_repr(source,
                              cmds_after_breakpoint=cmds_after_breakpoint)
        if exprepr:
            if gdb_repr == exprepr:
                # gdb managed to print the value in spite of the corruption;
                # this is good (see http://bugs.python.org/issue8330)
                return

        # Match anything for the type name; 0xDEADBEEF could point to
        # something arbitrary (see  http://bugs.python.org/issue8330)
        pattern = '<.* at remote 0x-?[0-9a-f]+>'

        m = re.match(pattern, gdb_repr)
        if not m:
            self.fail('Unexpected gdb representation: %r\n%s' % \
                          (gdb_repr, gdb_output))

    def test_NULL_ptr(self):
        'Ensure that a NULL PyObject* is handled gracefully'
        gdb_repr, gdb_output = (
            self.get_gdb_repr('id(42)',
                              cmds_after_breakpoint=['set variable v=0',
                                                     'backtrace'])
            )

        self.assertEqual(gdb_repr, '0x0')

    def test_NULL_ob_type(self):
        'Ensure that a PyObject* with NULL ob_type is handled gracefully'
        self.assertSane('id(42)',
                        'set v->ob_type=0')

    def test_corrupt_ob_type(self):
        'Ensure that a PyObject* with a corrupt ob_type is handled gracefully'
        self.assertSane('id(42)',
                        'set v->ob_type=0xDEADBEEF',
                        exprepr='42')

    def test_corrupt_tp_flags(self):
        'Ensure that a PyObject* with a type with corrupt tp_flags is handled'
        self.assertSane('id(42)',
                        'set v->ob_type->tp_flags=0x0',
                        exprepr='42')

    def test_corrupt_tp_name(self):
        'Ensure that a PyObject* with a type with corrupt tp_name is handled'
        self.assertSane('id(42)',
                        'set v->ob_type->tp_name=0xDEADBEEF',
                        exprepr='42')

    def test_builtins_help(self):
        'Ensure that the new-style class _Helper in site.py can be handled'
        # (this was the issue causing tracebacks in
        #  http://bugs.python.org/issue8032#msg100537 )
        gdb_repr, gdb_output = self.get_gdb_repr('id(__builtins__.help)', import_site=True)

        m = re.match(r'<_Helper at remote 0x-?[0-9a-f]+>', gdb_repr)
        self.assertTrue(m,
                        msg='Unexpected rendering %r' % gdb_repr)

    def test_selfreferential_list(self):
        '''Ensure that a reference loop involving a list doesn't lead proxyval
        into an infinite loop:'''
        gdb_repr, gdb_output = \
            self.get_gdb_repr("a = [3, 4, 5] ; a.append(a) ; id(a)")
        self.assertEqual(gdb_repr, '[3, 4, 5, [...]]')

        gdb_repr, gdb_output = \
            self.get_gdb_repr("a = [3, 4, 5] ; b = [a] ; a.append(b) ; id(a)")
        self.assertEqual(gdb_repr, '[3, 4, 5, [[...]]]')

    def test_selfreferential_dict(self):
        '''Ensure that a reference loop involving a dict doesn't lead proxyval
        into an infinite loop:'''
        gdb_repr, gdb_output = \
            self.get_gdb_repr("a = {} ; b = {'bar':a} ; a['foo'] = b ; id(a)")

        self.assertEqual(gdb_repr, "{'foo': {'bar': {...}}}")

    def test_selfreferential_old_style_instance(self):
        gdb_repr, gdb_output = \
            self.get_gdb_repr('''
class Foo:
    pass
foo = Foo()
foo.an_attr = foo
id(foo)''')
        self.assertTrue(re.match('<Foo\(an_attr=<\.\.\.>\) at remote 0x-?[0-9a-f]+>',
                                 gdb_repr),
                        'Unexpected gdb representation: %r\n%s' % \
                            (gdb_repr, gdb_output))

    def test_selfreferential_new_style_instance(self):
        gdb_repr, gdb_output = \
            self.get_gdb_repr('''
class Foo(object):
    pass
foo = Foo()
foo.an_attr = foo
id(foo)''')
        self.assertTrue(re.match('<Foo\(an_attr=<\.\.\.>\) at remote 0x-?[0-9a-f]+>',
                                 gdb_repr),
                        'Unexpected gdb representation: %r\n%s' % \
                            (gdb_repr, gdb_output))

        gdb_repr, gdb_output = \
            self.get_gdb_repr('''
class Foo(object):
    pass
a = Foo()
b = Foo()
a.an_attr = b
b.an_attr = a
id(a)''')
        self.assertTrue(re.match('<Foo\(an_attr=<Foo\(an_attr=<\.\.\.>\) at remote 0x-?[0-9a-f]+>\) at remote 0x-?[0-9a-f]+>',
                                 gdb_repr),
                        'Unexpected gdb representation: %r\n%s' % \
                            (gdb_repr, gdb_output))

    def test_truncation(self):
        'Verify that very long output is truncated'
        gdb_repr, gdb_output = self.get_gdb_repr('id(list(range(1000)))')
        self.assertEqual(gdb_repr,
                         "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, "
                         "14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, "
                         "27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, "
                         "40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, "
                         "53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, "
                         "66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, "
                         "79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, "
                         "92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, "
                         "104, 105, 106, 107, 108, 109, 110, 111, 112, 113, "
                         "114, 115, 116, 117, 118, 119, 120, 121, 122, 123, "
                         "124, 125, 126, 127, 128, 129, 130, 131, 132, 133, "
                         "134, 135, 136, 137, 138, 139, 140, 141, 142, 143, "
                         "144, 145, 146, 147, 148, 149, 150, 151, 152, 153, "
                         "154, 155, 156, 157, 158, 159, 160, 161, 162, 163, "
                         "164, 165, 166, 167, 168, 169, 170, 171, 172, 173, "
                         "174, 175, 176, 177, 178, 179, 180, 181, 182, 183, "
                         "184, 185, 186, 187, 188, 189, 190, 191, 192, 193, "
                         "194, 195, 196, 197, 198, 199, 200, 201, 202, 203, "
                         "204, 205, 206, 207, 208, 209, 210, 211, 212, 213, "
                         "214, 215, 216, 217, 218, 219, 220, 221, 222, 223, "
                         "224, 225, 226...(truncated)")
        self.assertEqual(len(gdb_repr),
                         1024 + len('...(truncated)'))

    def test_builtin_method(self):
        gdb_repr, gdb_output = self.get_gdb_repr('import sys; id(sys.stdout.readlines)')
        self.assertTrue(re.match('<built-in method readlines of _io.TextIOWrapper object at remote 0x-?[0-9a-f]+>',
                                 gdb_repr),
                        'Unexpected gdb representation: %r\n%s' % \
                            (gdb_repr, gdb_output))

    def test_frames(self):
        gdb_output = self.get_stack_trace('''
def foo(a, b, c):
    pass

foo(3, 4, 5)
id(foo.__code__)''',
                                          breakpoint='builtin_id',
                                          cmds_after_breakpoint=['print (PyFrameObject*)(((PyCodeObject*)v)->co_zombieframe)']
                                          )
        self.assertTrue(re.match('.*\s+\$1 =\s+Frame 0x-?[0-9a-f]+, for file <string>, line 3, in foo \(\)\s+.*',
                                 gdb_output,
                                 re.DOTALL),
                        'Unexpected gdb representation: %r\n%s' % (gdb_output, gdb_output))

@unittest.skipIf(python_is_optimized(),
                 "Python was compiled with optimizations")
class PyListTests(DebuggerTests):
    def assertListing(self, expected, actual):
        self.assertEndsWith(actual, expected)

    def test_basic_command(self):
        'Verify that the "py-list" command works'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-list'])

        self.assertListing('   5    \n'
                           '   6    def bar(a, b, c):\n'
                           '   7        baz(a, b, c)\n'
                           '   8    \n'
                           '   9    def baz(*args):\n'
                           ' >10        id(42)\n'
                           '  11    \n'
                           '  12    foo(1, 2, 3)\n',
                           bt)

    def test_one_abs_arg(self):
        'Verify the "py-list" command with one absolute argument'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-list 9'])

        self.assertListing('   9    def baz(*args):\n'
                           ' >10        id(42)\n'
                           '  11    \n'
                           '  12    foo(1, 2, 3)\n',
                           bt)

    def test_two_abs_args(self):
        'Verify the "py-list" command with two absolute arguments'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-list 1,3'])

        self.assertListing('   1    # Sample script for use by test_gdb.py\n'
                           '   2    \n'
                           '   3    def foo(a, b, c):\n',
                           bt)

class StackNavigationTests(DebuggerTests):
    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_pyup_command(self):
        'Verify that the "py-up" command works'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-up'])
        self.assertMultilineMatches(bt,
                                    r'''^.*
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 7, in bar \(a=1, b=2, c=3\)
    baz\(a, b, c\)
$''')

    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    def test_down_at_bottom(self):
        'Verify handling of "py-down" at the bottom of the stack'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-down'])
        self.assertEndsWith(bt,
                            'Unable to find a newer python frame\n')

    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    def test_up_at_top(self):
        'Verify handling of "py-up" at the top of the stack'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-up'] * 4)
        self.assertEndsWith(bt,
                            'Unable to find an older python frame\n')

    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_up_then_down(self):
        'Verify "py-up" followed by "py-down"'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-up', 'py-down'])
        self.assertMultilineMatches(bt,
                                    r'''^.*
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 7, in bar \(a=1, b=2, c=3\)
    baz\(a, b, c\)
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 10, in baz \(args=\(1, 2, 3\)\)
    id\(42\)
$''')

class PyBtTests(DebuggerTests):
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_bt(self):
        'Verify that the "py-bt" command works'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-bt'])
        self.assertMultilineMatches(bt,
                                    r'''^.*
Traceback \(most recent call first\):
  File ".*gdb_sample.py", line 10, in baz
    id\(42\)
  File ".*gdb_sample.py", line 7, in bar
    baz\(a, b, c\)
  File ".*gdb_sample.py", line 4, in foo
    bar\(a, b, c\)
  File ".*gdb_sample.py", line 12, in <module>
    foo\(1, 2, 3\)
''')

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_bt_full(self):
        'Verify that the "py-bt-full" command works'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-bt-full'])
        self.assertMultilineMatches(bt,
                                    r'''^.*
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 7, in bar \(a=1, b=2, c=3\)
    baz\(a, b, c\)
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 4, in foo \(a=1, b=2, c=3\)
    bar\(a, b, c\)
#[0-9]+ Frame 0x-?[0-9a-f]+, for file .*gdb_sample.py, line 12, in <module> \(\)
    foo\(1, 2, 3\)
''')

    @unittest.skipUnless(_thread,
                         "Python was compiled without thread support")
    def test_threads(self):
        'Verify that "py-bt" indicates threads that are waiting for the GIL'
        cmd = '''
from threading import Thread

class TestThread(Thread):
    # These threads would run forever, but we'll interrupt things with the
    # debugger
    def run(self):
        i = 0
        while 1:
             i += 1

t = {}
for i in range(4):
   t[i] = TestThread()
   t[i].start()

# Trigger a breakpoint on the main thread
id(42)

'''
        # Verify with "py-bt":
        gdb_output = self.get_stack_trace(cmd,
                                          cmds_after_breakpoint=['thread apply all py-bt'])
        self.assertIn('Waiting for the GIL', gdb_output)

        # Verify with "py-bt-full":
        gdb_output = self.get_stack_trace(cmd,
                                          cmds_after_breakpoint=['thread apply all py-bt-full'])
        self.assertIn('Waiting for the GIL', gdb_output)

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    # Some older versions of gdb will fail with
    #  "Cannot find new threads: generic error"
    # unless we add LD_PRELOAD=PATH-TO-libpthread.so.1 as a workaround
    @unittest.skipUnless(_thread,
                         "Python was compiled without thread support")
    def test_gc(self):
        'Verify that "py-bt" indicates if a thread is garbage-collecting'
        cmd = ('from gc import collect\n'
               'id(42)\n'
               'def foo():\n'
               '    collect()\n'
               'def bar():\n'
               '    foo()\n'
               'bar()\n')
        # Verify with "py-bt":
        gdb_output = self.get_stack_trace(cmd,
                                          cmds_after_breakpoint=['break update_refs', 'continue', 'py-bt'],
                                          )
        self.assertIn('Garbage-collecting', gdb_output)

        # Verify with "py-bt-full":
        gdb_output = self.get_stack_trace(cmd,
                                          cmds_after_breakpoint=['break update_refs', 'continue', 'py-bt-full'],
                                          )
        self.assertIn('Garbage-collecting', gdb_output)

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    # Some older versions of gdb will fail with
    #  "Cannot find new threads: generic error"
    # unless we add LD_PRELOAD=PATH-TO-libpthread.so.1 as a workaround
    @unittest.skipUnless(_thread,
                         "Python was compiled without thread support")
    def test_pycfunction(self):
        'Verify that "py-bt" displays invocations of PyCFunction instances'
        cmd = ('from time import sleep\n'
               'def foo():\n'
               '    sleep(1)\n'
               'def bar():\n'
               '    foo()\n'
               'bar()\n')
        # Verify with "py-bt":
        gdb_output = self.get_stack_trace(cmd,
                                          breakpoint='time_sleep',
                                          cmds_after_breakpoint=['bt', 'py-bt'],
                                          )
        self.assertIn('<built-in method sleep', gdb_output)

        # Verify with "py-bt-full":
        gdb_output = self.get_stack_trace(cmd,
                                          breakpoint='time_sleep',
                                          cmds_after_breakpoint=['py-bt-full'],
                                          )
        self.assertIn('#0 <built-in method sleep', gdb_output)


class PyPrintTests(DebuggerTests):
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_basic_command(self):
        'Verify that the "py-print" command works'
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-print args'])
        self.assertMultilineMatches(bt,
                                    r".*\nlocal 'args' = \(1, 2, 3\)\n.*")

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    def test_print_after_up(self):
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-up', 'py-print c', 'py-print b', 'py-print a'])
        self.assertMultilineMatches(bt,
                                    r".*\nlocal 'c' = 3\nlocal 'b' = 2\nlocal 'a' = 1\n.*")

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_printing_global(self):
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-print __name__'])
        self.assertMultilineMatches(bt,
                                    r".*\nglobal '__name__' = '__main__'\n.*")

    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_printing_builtin(self):
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-print len'])
        self.assertMultilineMatches(bt,
                                    r".*\nbuiltin 'len' = <built-in method len of module object at remote 0x-?[0-9a-f]+>\n.*")

class PyLocalsTests(DebuggerTests):
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_basic_command(self):
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-locals'])
        self.assertMultilineMatches(bt,
                                    r".*\nargs = \(1, 2, 3\)\n.*")

    @unittest.skipUnless(HAS_PYUP_PYDOWN, "test requires py-up/py-down commands")
    @unittest.skipIf(python_is_optimized(),
                     "Python was compiled with optimizations")
    def test_locals_after_up(self):
        bt = self.get_stack_trace(script=self.get_sample_script(),
                                  cmds_after_breakpoint=['py-up', 'py-locals'])
        self.assertMultilineMatches(bt,
                                    r".*\na = 1\nb = 2\nc = 3\n.*")

def test_main():
    if support.verbose:
        print("GDB version:")
        for line in os.fsdecode(gdb_version).splitlines():
            print(" " * 4 + line)
    run_unittest(PrettyPrintTests,
                 PyListTests,
                 StackNavigationTests,
                 PyBtTests,
                 PyPrintTests,
                 PyLocalsTests
                 )

if __name__ == "__main__":
    test_main()
