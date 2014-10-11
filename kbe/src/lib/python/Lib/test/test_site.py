"""Tests for 'site'.

Tests assume the initial paths in sys.path once the interpreter has begun
executing have not been removed.

"""
import unittest
import test.support
from test.support import captured_stderr, TESTFN, EnvironmentVarGuard
import builtins
import os
import sys
import re
import encodings
import urllib.request
import urllib.error
import subprocess
import sysconfig
from copy import copy

# These tests are not particularly useful if Python was invoked with -S.
# If you add tests that are useful under -S, this skip should be moved
# to the class level.
if sys.flags.no_site:
    raise unittest.SkipTest("Python was invoked with -S")

import site

if site.ENABLE_USER_SITE and not os.path.isdir(site.USER_SITE):
    # need to add user site directory for tests
    os.makedirs(site.USER_SITE)
    site.addsitedir(site.USER_SITE)

class HelperFunctionsTests(unittest.TestCase):
    """Tests for helper functions.
    """

    def setUp(self):
        """Save a copy of sys.path"""
        self.sys_path = sys.path[:]
        self.old_base = site.USER_BASE
        self.old_site = site.USER_SITE
        self.old_prefixes = site.PREFIXES
        self.original_vars = sysconfig._CONFIG_VARS
        self.old_vars = copy(sysconfig._CONFIG_VARS)

    def tearDown(self):
        """Restore sys.path"""
        sys.path[:] = self.sys_path
        site.USER_BASE = self.old_base
        site.USER_SITE = self.old_site
        site.PREFIXES = self.old_prefixes
        sysconfig._CONFIG_VARS = self.original_vars
        sysconfig._CONFIG_VARS.clear()
        sysconfig._CONFIG_VARS.update(self.old_vars)

    def test_makepath(self):
        # Test makepath() have an absolute path for its first return value
        # and a case-normalized version of the absolute path for its
        # second value.
        path_parts = ("Beginning", "End")
        original_dir = os.path.join(*path_parts)
        abs_dir, norm_dir = site.makepath(*path_parts)
        self.assertEqual(os.path.abspath(original_dir), abs_dir)
        if original_dir == os.path.normcase(original_dir):
            self.assertEqual(abs_dir, norm_dir)
        else:
            self.assertEqual(os.path.normcase(abs_dir), norm_dir)

    def test_init_pathinfo(self):
        dir_set = site._init_pathinfo()
        for entry in [site.makepath(path)[1] for path in sys.path
                        if path and os.path.isdir(path)]:
            self.assertIn(entry, dir_set,
                          "%s from sys.path not found in set returned "
                          "by _init_pathinfo(): %s" % (entry, dir_set))

    def pth_file_tests(self, pth_file):
        """Contain common code for testing results of reading a .pth file"""
        self.assertIn(pth_file.imported, sys.modules,
                      "%s not in sys.modules" % pth_file.imported)
        self.assertIn(site.makepath(pth_file.good_dir_path)[0], sys.path)
        self.assertFalse(os.path.exists(pth_file.bad_dir_path))

    def test_addpackage(self):
        # Make sure addpackage() imports if the line starts with 'import',
        # adds directories to sys.path for any line in the file that is not a
        # comment or import that is a valid directory name for where the .pth
        # file resides; invalid directories are not added
        pth_file = PthFile()
        pth_file.cleanup(prep=True)  # to make sure that nothing is
                                      # pre-existing that shouldn't be
        try:
            pth_file.create()
            site.addpackage(pth_file.base_dir, pth_file.filename, set())
            self.pth_file_tests(pth_file)
        finally:
            pth_file.cleanup()

    def make_pth(self, contents, pth_dir='.', pth_name=TESTFN):
        # Create a .pth file and return its (abspath, basename).
        pth_dir = os.path.abspath(pth_dir)
        pth_basename = pth_name + '.pth'
        pth_fn = os.path.join(pth_dir, pth_basename)
        pth_file = open(pth_fn, 'w', encoding='utf-8')
        self.addCleanup(lambda: os.remove(pth_fn))
        pth_file.write(contents)
        pth_file.close()
        return pth_dir, pth_basename

    def test_addpackage_import_bad_syntax(self):
        # Issue 10642
        pth_dir, pth_fn = self.make_pth("import bad)syntax\n")
        with captured_stderr() as err_out:
            site.addpackage(pth_dir, pth_fn, set())
        self.assertRegex(err_out.getvalue(), "line 1")
        self.assertRegex(err_out.getvalue(),
            re.escape(os.path.join(pth_dir, pth_fn)))
        # XXX: the previous two should be independent checks so that the
        # order doesn't matter.  The next three could be a single check
        # but my regex foo isn't good enough to write it.
        self.assertRegex(err_out.getvalue(), 'Traceback')
        self.assertRegex(err_out.getvalue(), r'import bad\)syntax')
        self.assertRegex(err_out.getvalue(), 'SyntaxError')

    def test_addpackage_import_bad_exec(self):
        # Issue 10642
        pth_dir, pth_fn = self.make_pth("randompath\nimport nosuchmodule\n")
        with captured_stderr() as err_out:
            site.addpackage(pth_dir, pth_fn, set())
        self.assertRegex(err_out.getvalue(), "line 2")
        self.assertRegex(err_out.getvalue(),
            re.escape(os.path.join(pth_dir, pth_fn)))
        # XXX: ditto previous XXX comment.
        self.assertRegex(err_out.getvalue(), 'Traceback')
        self.assertRegex(err_out.getvalue(), 'ImportError')

    @unittest.skipIf(sys.platform == "win32", "Windows does not raise an "
                      "error for file paths containing null characters")
    def test_addpackage_import_bad_pth_file(self):
        # Issue 5258
        pth_dir, pth_fn = self.make_pth("abc\x00def\n")
        with captured_stderr() as err_out:
            site.addpackage(pth_dir, pth_fn, set())
        self.assertRegex(err_out.getvalue(), "line 1")
        self.assertRegex(err_out.getvalue(),
            re.escape(os.path.join(pth_dir, pth_fn)))
        # XXX: ditto previous XXX comment.
        self.assertRegex(err_out.getvalue(), 'Traceback')
        self.assertRegex(err_out.getvalue(), 'TypeError')

    def test_addsitedir(self):
        # Same tests for test_addpackage since addsitedir() essentially just
        # calls addpackage() for every .pth file in the directory
        pth_file = PthFile()
        pth_file.cleanup(prep=True) # Make sure that nothing is pre-existing
                                    # that is tested for
        try:
            pth_file.create()
            site.addsitedir(pth_file.base_dir, set())
            self.pth_file_tests(pth_file)
        finally:
            pth_file.cleanup()

    @unittest.skipUnless(site.ENABLE_USER_SITE, "requires access to PEP 370 "
                          "user-site (site.ENABLE_USER_SITE)")
    def test_s_option(self):
        usersite = site.USER_SITE
        self.assertIn(usersite, sys.path)

        env = os.environ.copy()
        rc = subprocess.call([sys.executable, '-c',
            'import sys; sys.exit(%r in sys.path)' % usersite],
            env=env)
        self.assertEqual(rc, 1)

        env = os.environ.copy()
        rc = subprocess.call([sys.executable, '-s', '-c',
            'import sys; sys.exit(%r in sys.path)' % usersite],
            env=env)
        if usersite == site.getsitepackages()[0]:
            self.assertEqual(rc, 1)
        else:
            self.assertEqual(rc, 0)

        env = os.environ.copy()
        env["PYTHONNOUSERSITE"] = "1"
        rc = subprocess.call([sys.executable, '-c',
            'import sys; sys.exit(%r in sys.path)' % usersite],
            env=env)
        if usersite == site.getsitepackages()[0]:
            self.assertEqual(rc, 1)
        else:
            self.assertEqual(rc, 0)

        env = os.environ.copy()
        env["PYTHONUSERBASE"] = "/tmp"
        rc = subprocess.call([sys.executable, '-c',
            'import sys, site; sys.exit(site.USER_BASE.startswith("/tmp"))'],
            env=env)
        self.assertEqual(rc, 1)

    def test_getuserbase(self):
        site.USER_BASE = None
        user_base = site.getuserbase()

        # the call sets site.USER_BASE
        self.assertEqual(site.USER_BASE, user_base)

        # let's set PYTHONUSERBASE and see if it uses it
        site.USER_BASE = None
        import sysconfig
        sysconfig._CONFIG_VARS = None

        with EnvironmentVarGuard() as environ:
            environ['PYTHONUSERBASE'] = 'xoxo'
            self.assertTrue(site.getuserbase().startswith('xoxo'),
                            site.getuserbase())

    def test_getusersitepackages(self):
        site.USER_SITE = None
        site.USER_BASE = None
        user_site = site.getusersitepackages()

        # the call sets USER_BASE *and* USER_SITE
        self.assertEqual(site.USER_SITE, user_site)
        self.assertTrue(user_site.startswith(site.USER_BASE), user_site)

    def test_getsitepackages(self):
        site.PREFIXES = ['xoxo']
        dirs = site.getsitepackages()

        if (sys.platform == "darwin" and
            sysconfig.get_config_var("PYTHONFRAMEWORK")):
            # OS X framework builds
            site.PREFIXES = ['Python.framework']
            dirs = site.getsitepackages()
            self.assertEqual(len(dirs), 3)
            wanted = os.path.join('/Library',
                                  sysconfig.get_config_var("PYTHONFRAMEWORK"),
                                  sys.version[:3],
                                  'site-packages')
            self.assertEqual(dirs[2], wanted)
        elif os.sep == '/':
            # OS X non-framwework builds, Linux, FreeBSD, etc
            self.assertEqual(len(dirs), 2)
            wanted = os.path.join('xoxo', 'lib', 'python' + sys.version[:3],
                                  'site-packages')
            self.assertEqual(dirs[0], wanted)
            wanted = os.path.join('xoxo', 'lib', 'site-python')
            self.assertEqual(dirs[1], wanted)
        else:
            # other platforms
            self.assertEqual(len(dirs), 2)
            self.assertEqual(dirs[0], 'xoxo')
            wanted = os.path.join('xoxo', 'lib', 'site-packages')
            self.assertEqual(dirs[1], wanted)

class PthFile(object):
    """Helper class for handling testing of .pth files"""

    def __init__(self, filename_base=TESTFN, imported="time",
                    good_dirname="__testdir__", bad_dirname="__bad"):
        """Initialize instance variables"""
        self.filename = filename_base + ".pth"
        self.base_dir = os.path.abspath('')
        self.file_path = os.path.join(self.base_dir, self.filename)
        self.imported = imported
        self.good_dirname = good_dirname
        self.bad_dirname = bad_dirname
        self.good_dir_path = os.path.join(self.base_dir, self.good_dirname)
        self.bad_dir_path = os.path.join(self.base_dir, self.bad_dirname)

    def create(self):
        """Create a .pth file with a comment, blank lines, an ``import
        <self.imported>``, a line with self.good_dirname, and a line with
        self.bad_dirname.

        Creation of the directory for self.good_dir_path (based off of
        self.good_dirname) is also performed.

        Make sure to call self.cleanup() to undo anything done by this method.

        """
        FILE = open(self.file_path, 'w')
        try:
            print("#import @bad module name", file=FILE)
            print("\n", file=FILE)
            print("import %s" % self.imported, file=FILE)
            print(self.good_dirname, file=FILE)
            print(self.bad_dirname, file=FILE)
        finally:
            FILE.close()
        os.mkdir(self.good_dir_path)

    def cleanup(self, prep=False):
        """Make sure that the .pth file is deleted, self.imported is not in
        sys.modules, and that both self.good_dirname and self.bad_dirname are
        not existing directories."""
        if os.path.exists(self.file_path):
            os.remove(self.file_path)
        if prep:
            self.imported_module = sys.modules.get(self.imported)
            if self.imported_module:
                del sys.modules[self.imported]
        else:
            if self.imported_module:
                sys.modules[self.imported] = self.imported_module
        if os.path.exists(self.good_dir_path):
            os.rmdir(self.good_dir_path)
        if os.path.exists(self.bad_dir_path):
            os.rmdir(self.bad_dir_path)

class ImportSideEffectTests(unittest.TestCase):
    """Test side-effects from importing 'site'."""

    def setUp(self):
        """Make a copy of sys.path"""
        self.sys_path = sys.path[:]

    def tearDown(self):
        """Restore sys.path"""
        sys.path[:] = self.sys_path

    def test_abs_paths(self):
        # Make sure all imported modules have their __file__ and __cached__
        # attributes as absolute paths.  Arranging to put the Lib directory on
        # PYTHONPATH would cause the os module to have a relative path for
        # __file__ if abs_paths() does not get run.  sys and builtins (the
        # only other modules imported before site.py runs) do not have
        # __file__ or __cached__ because they are built-in.
        parent = os.path.relpath(os.path.dirname(os.__file__))
        env = os.environ.copy()
        env['PYTHONPATH'] = parent
        code = ('import os, sys',
            # use ASCII to avoid locale issues with non-ASCII directories
            'os_file = os.__file__.encode("ascii", "backslashreplace")',
            r'sys.stdout.buffer.write(os_file + b"\n")',
            'os_cached = os.__cached__.encode("ascii", "backslashreplace")',
            r'sys.stdout.buffer.write(os_cached + b"\n")')
        command = '\n'.join(code)
        # First, prove that with -S (no 'import site'), the paths are
        # relative.
        proc = subprocess.Popen([sys.executable, '-S', '-c', command],
                                env=env,
                                stdout=subprocess.PIPE)
        stdout, stderr = proc.communicate()

        self.assertEqual(proc.returncode, 0)
        os__file__, os__cached__ = stdout.splitlines()[:2]
        self.assertFalse(os.path.isabs(os__file__))
        self.assertFalse(os.path.isabs(os__cached__))
        # Now, with 'import site', it works.
        proc = subprocess.Popen([sys.executable, '-c', command],
                                env=env,
                                stdout=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        self.assertEqual(proc.returncode, 0)
        os__file__, os__cached__ = stdout.splitlines()[:2]
        self.assertTrue(os.path.isabs(os__file__))
        self.assertTrue(os.path.isabs(os__cached__))

    def test_no_duplicate_paths(self):
        # No duplicate paths should exist in sys.path
        # Handled by removeduppaths()
        site.removeduppaths()
        seen_paths = set()
        for path in sys.path:
            self.assertNotIn(path, seen_paths)
            seen_paths.add(path)

    @unittest.skip('test not implemented')
    def test_add_build_dir(self):
        # Test that the build directory's Modules directory is used when it
        # should be.
        # XXX: implement
        pass

    def test_setting_quit(self):
        # 'quit' and 'exit' should be injected into builtins
        self.assertTrue(hasattr(builtins, "quit"))
        self.assertTrue(hasattr(builtins, "exit"))

    def test_setting_copyright(self):
        # 'copyright', 'credits', and 'license' should be in builtins
        self.assertTrue(hasattr(builtins, "copyright"))
        self.assertTrue(hasattr(builtins, "credits"))
        self.assertTrue(hasattr(builtins, "license"))

    def test_setting_help(self):
        # 'help' should be set in builtins
        self.assertTrue(hasattr(builtins, "help"))

    def test_aliasing_mbcs(self):
        if sys.platform == "win32":
            import locale
            if locale.getdefaultlocale()[1].startswith('cp'):
                for value in encodings.aliases.aliases.values():
                    if value == "mbcs":
                        break
                else:
                    self.fail("did not alias mbcs")

    def test_sitecustomize_executed(self):
        # If sitecustomize is available, it should have been imported.
        if "sitecustomize" not in sys.modules:
            try:
                import sitecustomize
            except ImportError:
                pass
            else:
                self.fail("sitecustomize not imported automatically")

    @test.support.requires_resource('network')
    @unittest.skipUnless(sys.version_info[3] == 'final',
                         'only for released versions')
    @unittest.skipUnless(hasattr(urllib.request, "HTTPSHandler"),
                         'need SSL support to download license')
    def test_license_exists_at_url(self):
        # This test is a bit fragile since it depends on the format of the
        # string displayed by license in the absence of a LICENSE file.
        url = license._Printer__data.split()[1]
        req = urllib.request.Request(url, method='HEAD')
        try:
            with test.support.transient_internet(url):
                with urllib.request.urlopen(req) as data:
                    code = data.getcode()
        except urllib.error.HTTPError as e:
            code = e.code
        self.assertEqual(code, 200, msg="Can't find " + url)


class StartupImportTests(unittest.TestCase):

    def test_startup_imports(self):
        # This tests checks which modules are loaded by Python when it
        # initially starts upon startup.
        popen = subprocess.Popen([sys.executable, '-I', '-v', '-c',
                                  'import sys; print(set(sys.modules))'],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
        stdout, stderr = popen.communicate()
        stdout = stdout.decode('utf-8')
        stderr = stderr.decode('utf-8')
        modules = eval(stdout)

        self.assertIn('site', modules)

        # http://bugs.python.org/issue19205
        re_mods = {'re', '_sre', 'sre_compile', 'sre_constants', 'sre_parse'}
        # _osx_support uses the re module in many placs
        if sys.platform != 'darwin':
            self.assertFalse(modules.intersection(re_mods), stderr)
        # http://bugs.python.org/issue9548
        self.assertNotIn('locale', modules, stderr)
        if sys.platform != 'darwin':
            # http://bugs.python.org/issue19209
            self.assertNotIn('copyreg', modules, stderr)
        # http://bugs.python.org/issue19218>
        collection_mods = {'_collections', 'collections', 'functools',
                           'heapq', 'itertools', 'keyword', 'operator',
                           'reprlib', 'types', 'weakref'
                          }.difference(sys.builtin_module_names)
        self.assertFalse(modules.intersection(collection_mods), stderr)


if __name__ == "__main__":
    unittest.main()
