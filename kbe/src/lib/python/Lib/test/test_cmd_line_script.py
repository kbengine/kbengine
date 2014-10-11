# tests command line execution of scripts

import contextlib
import importlib
import importlib.machinery
import zipimport
import unittest
import sys
import os
import os.path
import py_compile
import subprocess

import textwrap
from test import support
from test.script_helper import (
    make_pkg, make_script, make_zip_pkg, make_zip_script,
    assert_python_ok, assert_python_failure, temp_dir,
    spawn_python, kill_python)

verbose = support.verbose

example_args = ['test1', 'test2', 'test3']

test_source = """\
# Script may be run with optimisation enabled, so don't rely on assert
# statements being executed
def assertEqual(lhs, rhs):
    if lhs != rhs:
        raise AssertionError('%r != %r' % (lhs, rhs))
def assertIdentical(lhs, rhs):
    if lhs is not rhs:
        raise AssertionError('%r is not %r' % (lhs, rhs))
# Check basic code execution
result = ['Top level assignment']
def f():
    result.append('Lower level reference')
f()
assertEqual(result, ['Top level assignment', 'Lower level reference'])
# Check population of magic variables
assertEqual(__name__, '__main__')
from importlib.machinery import BuiltinImporter
_loader = __loader__ if __loader__ is BuiltinImporter else type(__loader__)
print('__loader__==%a' % _loader)
print('__file__==%a' % __file__)
print('__cached__==%a' % __cached__)
print('__package__==%r' % __package__)
# Check PEP 451 details
import os.path
if __package__ is not None:
    print('__main__ was located through the import system')
    assertIdentical(__spec__.loader, __loader__)
    expected_spec_name = os.path.splitext(os.path.basename(__file__))[0]
    if __package__:
        expected_spec_name = __package__ + "." + expected_spec_name
    assertEqual(__spec__.name, expected_spec_name)
    assertEqual(__spec__.parent, __package__)
    assertIdentical(__spec__.submodule_search_locations, None)
    assertEqual(__spec__.origin, __file__)
    if __spec__.cached is not None:
        assertEqual(__spec__.cached, __cached__)
# Check the sys module
import sys
assertIdentical(globals(), sys.modules[__name__].__dict__)
if __spec__ is not None:
    # XXX: We're not currently making __main__ available under its real name
    pass # assertIdentical(globals(), sys.modules[__spec__.name].__dict__)
from test import test_cmd_line_script
example_args_list = test_cmd_line_script.example_args
assertEqual(sys.argv[1:], example_args_list)
print('sys.argv[0]==%a' % sys.argv[0])
print('sys.path[0]==%a' % sys.path[0])
# Check the working directory
import os
print('cwd==%a' % os.getcwd())
"""

def _make_test_script(script_dir, script_basename, source=test_source):
    to_return = make_script(script_dir, script_basename, source)
    importlib.invalidate_caches()
    return to_return

def _make_test_zip_pkg(zip_dir, zip_basename, pkg_name, script_basename,
                       source=test_source, depth=1):
    to_return = make_zip_pkg(zip_dir, zip_basename, pkg_name, script_basename,
                             source, depth)
    importlib.invalidate_caches()
    return to_return

# There's no easy way to pass the script directory in to get
# -m to work (avoiding that is the whole point of making
# directories and zipfiles executable!)
# So we fake it for testing purposes with a custom launch script
launch_source = """\
import sys, os.path, runpy
sys.path.insert(0, %s)
runpy._run_module_as_main(%r)
"""

def _make_launch_script(script_dir, script_basename, module_name, path=None):
    if path is None:
        path = "os.path.dirname(__file__)"
    else:
        path = repr(path)
    source = launch_source % (path, module_name)
    to_return = make_script(script_dir, script_basename, source)
    importlib.invalidate_caches()
    return to_return

class CmdLineTest(unittest.TestCase):
    def _check_output(self, script_name, exit_code, data,
                             expected_file, expected_argv0,
                             expected_path0, expected_package,
                             expected_loader):
        if verbose > 1:
            print("Output from test script %r:" % script_name)
            print(data)
        self.assertEqual(exit_code, 0)
        printed_loader = '__loader__==%a' % expected_loader
        printed_file = '__file__==%a' % expected_file
        printed_package = '__package__==%r' % expected_package
        printed_argv0 = 'sys.argv[0]==%a' % expected_argv0
        printed_path0 = 'sys.path[0]==%a' % expected_path0
        printed_cwd = 'cwd==%a' % os.getcwd()
        if verbose > 1:
            print('Expected output:')
            print(printed_file)
            print(printed_package)
            print(printed_argv0)
            print(printed_cwd)
        self.assertIn(printed_loader.encode('utf-8'), data)
        self.assertIn(printed_file.encode('utf-8'), data)
        self.assertIn(printed_package.encode('utf-8'), data)
        self.assertIn(printed_argv0.encode('utf-8'), data)
        self.assertIn(printed_path0.encode('utf-8'), data)
        self.assertIn(printed_cwd.encode('utf-8'), data)

    def _check_script(self, script_name, expected_file,
                            expected_argv0, expected_path0,
                            expected_package, expected_loader,
                            *cmd_line_switches):
        if not __debug__:
            cmd_line_switches += ('-' + 'O' * sys.flags.optimize,)
        run_args = cmd_line_switches + (script_name,) + tuple(example_args)
        rc, out, err = assert_python_ok(*run_args, __isolated=False)
        self._check_output(script_name, rc, out + err, expected_file,
                           expected_argv0, expected_path0,
                           expected_package, expected_loader)

    def _check_import_error(self, script_name, expected_msg,
                            *cmd_line_switches):
        run_args = cmd_line_switches + (script_name,)
        rc, out, err = assert_python_failure(*run_args)
        if verbose > 1:
            print('Output from test script %r:' % script_name)
            print(err)
            print('Expected output: %r' % expected_msg)
        self.assertIn(expected_msg.encode('utf-8'), err)

    def test_dash_c_loader(self):
        rc, out, err = assert_python_ok("-c", "print(__loader__)")
        expected = repr(importlib.machinery.BuiltinImporter).encode("utf-8")
        self.assertIn(expected, out)

    def test_stdin_loader(self):
        # Unfortunately, there's no way to automatically test the fully
        # interactive REPL, since that code path only gets executed when
        # stdin is an interactive tty.
        p = spawn_python()
        try:
            p.stdin.write(b"print(__loader__)\n")
            p.stdin.flush()
        finally:
            out = kill_python(p)
        expected = repr(importlib.machinery.BuiltinImporter).encode("utf-8")
        self.assertIn(expected, out)

    @contextlib.contextmanager
    def interactive_python(self, separate_stderr=False):
        if separate_stderr:
            p = spawn_python('-i', bufsize=1, stderr=subprocess.PIPE)
            stderr = p.stderr
        else:
            p = spawn_python('-i', bufsize=1, stderr=subprocess.STDOUT)
            stderr = p.stdout
        try:
            # Drain stderr until prompt
            while True:
                data = stderr.read(4)
                if data == b">>> ":
                    break
                stderr.readline()
            yield p
        finally:
            kill_python(p)
            stderr.close()

    def check_repl_stdout_flush(self, separate_stderr=False):
        with self.interactive_python(separate_stderr) as p:
            p.stdin.write(b"print('foo')\n")
            p.stdin.flush()
            self.assertEqual(b'foo', p.stdout.readline().strip())

    def check_repl_stderr_flush(self, separate_stderr=False):
        with self.interactive_python(separate_stderr) as p:
            p.stdin.write(b"1/0\n")
            p.stdin.flush()
            stderr = p.stderr if separate_stderr else p.stdout
            self.assertIn(b'Traceback ', stderr.readline())
            self.assertIn(b'File "<stdin>"', stderr.readline())
            self.assertIn(b'ZeroDivisionError', stderr.readline())

    def test_repl_stdout_flush(self):
        self.check_repl_stdout_flush()

    def test_repl_stdout_flush_separate_stderr(self):
        self.check_repl_stdout_flush(True)

    def test_repl_stderr_flush(self):
        self.check_repl_stderr_flush()

    def test_repl_stderr_flush_separate_stderr(self):
        self.check_repl_stderr_flush(True)

    def test_basic_script(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'script')
            self._check_script(script_name, script_name, script_name,
                               script_dir, None,
                               importlib.machinery.SourceFileLoader)

    def test_script_compiled(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'script')
            py_compile.compile(script_name, doraise=True)
            os.remove(script_name)
            pyc_file = support.make_legacy_pyc(script_name)
            self._check_script(pyc_file, pyc_file,
                               pyc_file, script_dir, None,
                               importlib.machinery.SourcelessFileLoader)

    def test_directory(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, '__main__')
            self._check_script(script_dir, script_name, script_dir,
                               script_dir, '',
                               importlib.machinery.SourceFileLoader)

    def test_directory_compiled(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, '__main__')
            py_compile.compile(script_name, doraise=True)
            os.remove(script_name)
            pyc_file = support.make_legacy_pyc(script_name)
            self._check_script(script_dir, pyc_file, script_dir,
                               script_dir, '',
                               importlib.machinery.SourcelessFileLoader)

    def test_directory_error(self):
        with temp_dir() as script_dir:
            msg = "can't find '__main__' module in %r" % script_dir
            self._check_import_error(script_dir, msg)

    def test_zipfile(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, '__main__')
            zip_name, run_name = make_zip_script(script_dir, 'test_zip', script_name)
            self._check_script(zip_name, run_name, zip_name, zip_name, '',
                               zipimport.zipimporter)

    def test_zipfile_compiled(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, '__main__')
            compiled_name = py_compile.compile(script_name, doraise=True)
            zip_name, run_name = make_zip_script(script_dir, 'test_zip', compiled_name)
            self._check_script(zip_name, run_name, zip_name, zip_name, '',
                               zipimport.zipimporter)

    def test_zipfile_error(self):
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'not_main')
            zip_name, run_name = make_zip_script(script_dir, 'test_zip', script_name)
            msg = "can't find '__main__' module in %r" % zip_name
            self._check_import_error(zip_name, msg)

    def test_module_in_package(self):
        with temp_dir() as script_dir:
            pkg_dir = os.path.join(script_dir, 'test_pkg')
            make_pkg(pkg_dir)
            script_name = _make_test_script(pkg_dir, 'script')
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg.script')
            self._check_script(launch_name, script_name, script_name,
                               script_dir, 'test_pkg',
                               importlib.machinery.SourceFileLoader)

    def test_module_in_package_in_zipfile(self):
        with temp_dir() as script_dir:
            zip_name, run_name = _make_test_zip_pkg(script_dir, 'test_zip', 'test_pkg', 'script')
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg.script', zip_name)
            self._check_script(launch_name, run_name, run_name,
                               zip_name, 'test_pkg', zipimport.zipimporter)

    def test_module_in_subpackage_in_zipfile(self):
        with temp_dir() as script_dir:
            zip_name, run_name = _make_test_zip_pkg(script_dir, 'test_zip', 'test_pkg', 'script', depth=2)
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg.test_pkg.script', zip_name)
            self._check_script(launch_name, run_name, run_name,
                               zip_name, 'test_pkg.test_pkg',
                               zipimport.zipimporter)

    def test_package(self):
        with temp_dir() as script_dir:
            pkg_dir = os.path.join(script_dir, 'test_pkg')
            make_pkg(pkg_dir)
            script_name = _make_test_script(pkg_dir, '__main__')
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg')
            self._check_script(launch_name, script_name,
                               script_name, script_dir, 'test_pkg',
                               importlib.machinery.SourceFileLoader)

    def test_package_compiled(self):
        with temp_dir() as script_dir:
            pkg_dir = os.path.join(script_dir, 'test_pkg')
            make_pkg(pkg_dir)
            script_name = _make_test_script(pkg_dir, '__main__')
            compiled_name = py_compile.compile(script_name, doraise=True)
            os.remove(script_name)
            pyc_file = support.make_legacy_pyc(script_name)
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg')
            self._check_script(launch_name, pyc_file,
                               pyc_file, script_dir, 'test_pkg',
                               importlib.machinery.SourcelessFileLoader)

    def test_package_error(self):
        with temp_dir() as script_dir:
            pkg_dir = os.path.join(script_dir, 'test_pkg')
            make_pkg(pkg_dir)
            msg = ("'test_pkg' is a package and cannot "
                   "be directly executed")
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg')
            self._check_import_error(launch_name, msg)

    def test_package_recursion(self):
        with temp_dir() as script_dir:
            pkg_dir = os.path.join(script_dir, 'test_pkg')
            make_pkg(pkg_dir)
            main_dir = os.path.join(pkg_dir, '__main__')
            make_pkg(main_dir)
            msg = ("Cannot use package as __main__ module; "
                   "'test_pkg' is a package and cannot "
                   "be directly executed")
            launch_name = _make_launch_script(script_dir, 'launch', 'test_pkg')
            self._check_import_error(launch_name, msg)

    def test_issue8202(self):
        # Make sure package __init__ modules see "-m" in sys.argv0 while
        # searching for the module to execute
        with temp_dir() as script_dir:
            with support.change_cwd(path=script_dir):
                pkg_dir = os.path.join(script_dir, 'test_pkg')
                make_pkg(pkg_dir, "import sys; print('init_argv0==%r' % sys.argv[0])")
                script_name = _make_test_script(pkg_dir, 'script')
                rc, out, err = assert_python_ok('-m', 'test_pkg.script', *example_args, __isolated=False)
                if verbose > 1:
                    print(out)
                expected = "init_argv0==%r" % '-m'
                self.assertIn(expected.encode('utf-8'), out)
                self._check_output(script_name, rc, out,
                                   script_name, script_name, '', 'test_pkg',
                                   importlib.machinery.SourceFileLoader)

    def test_issue8202_dash_c_file_ignored(self):
        # Make sure a "-c" file in the current directory
        # does not alter the value of sys.path[0]
        with temp_dir() as script_dir:
            with support.change_cwd(path=script_dir):
                with open("-c", "w") as f:
                    f.write("data")
                    rc, out, err = assert_python_ok('-c',
                        'import sys; print("sys.path[0]==%r" % sys.path[0])',
                        __isolated=False)
                    if verbose > 1:
                        print(out)
                    expected = "sys.path[0]==%r" % ''
                    self.assertIn(expected.encode('utf-8'), out)

    def test_issue8202_dash_m_file_ignored(self):
        # Make sure a "-m" file in the current directory
        # does not alter the value of sys.path[0]
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'other')
            with support.change_cwd(path=script_dir):
                with open("-m", "w") as f:
                    f.write("data")
                    rc, out, err = assert_python_ok('-m', 'other', *example_args,
                                                    __isolated=False)
                    self._check_output(script_name, rc, out,
                                      script_name, script_name, '', '',
                                      importlib.machinery.SourceFileLoader)

    def test_dash_m_error_code_is_one(self):
        # If a module is invoked with the -m command line flag
        # and results in an error that the return code to the
        # shell is '1'
        with temp_dir() as script_dir:
            with support.change_cwd(path=script_dir):
                pkg_dir = os.path.join(script_dir, 'test_pkg')
                make_pkg(pkg_dir)
                script_name = _make_test_script(pkg_dir, 'other',
                                                "if __name__ == '__main__': raise ValueError")
                rc, out, err = assert_python_failure('-m', 'test_pkg.other', *example_args)
                if verbose > 1:
                    print(out)
                self.assertEqual(rc, 1)

    def test_pep_409_verbiage(self):
        # Make sure PEP 409 syntax properly suppresses
        # the context of an exception
        script = textwrap.dedent("""\
            try:
                raise ValueError
            except:
                raise NameError from None
            """)
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'script', script)
            exitcode, stdout, stderr = assert_python_failure(script_name)
            text = stderr.decode('ascii').split('\n')
            self.assertEqual(len(text), 4)
            self.assertTrue(text[0].startswith('Traceback'))
            self.assertTrue(text[1].startswith('  File '))
            self.assertTrue(text[3].startswith('NameError'))

    def test_non_ascii(self):
        # Mac OS X denies the creation of a file with an invalid UTF-8 name.
        # Windows allows to create a name with an arbitrary bytes name, but
        # Python cannot a undecodable bytes argument to a subprocess.
        if (support.TESTFN_UNDECODABLE
        and sys.platform not in ('win32', 'darwin')):
            name = os.fsdecode(support.TESTFN_UNDECODABLE)
        elif support.TESTFN_NONASCII:
            name = support.TESTFN_NONASCII
        else:
            self.skipTest("need support.TESTFN_NONASCII")

        # Issue #16218
        source = 'print(ascii(__file__))\n'
        script_name = _make_test_script(os.curdir, name, source)
        self.addCleanup(support.unlink, script_name)
        rc, stdout, stderr = assert_python_ok(script_name)
        self.assertEqual(
            ascii(script_name),
            stdout.rstrip().decode('ascii'),
            'stdout=%r stderr=%r' % (stdout, stderr))
        self.assertEqual(0, rc)

    def test_issue20500_exit_with_exception_value(self):
        script = textwrap.dedent("""\
            import sys
            error = None
            try:
                raise ValueError('some text')
            except ValueError as err:
                error = err

            if error:
                sys.exit(error)
            """)
        with temp_dir() as script_dir:
            script_name = _make_test_script(script_dir, 'script', script)
            exitcode, stdout, stderr = assert_python_failure(script_name)
            text = stderr.decode('ascii')
            self.assertEqual(text, "some text")


def test_main():
    support.run_unittest(CmdLineTest)
    support.reap_children()

if __name__ == '__main__':
    test_main()
