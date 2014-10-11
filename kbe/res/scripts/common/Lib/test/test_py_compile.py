import importlib.util
import os
import py_compile
import shutil
import stat
import sys
import tempfile
import unittest

from test import support, script_helper

class PyCompileTests(unittest.TestCase):

    def setUp(self):
        self.directory = tempfile.mkdtemp()
        self.source_path = os.path.join(self.directory, '_test.py')
        self.pyc_path = self.source_path + 'c'
        self.cache_path = importlib.util.cache_from_source(self.source_path)
        self.cwd_drive = os.path.splitdrive(os.getcwd())[0]
        # In these tests we compute relative paths.  When using Windows, the
        # current working directory path and the 'self.source_path' might be
        # on different drives.  Therefore we need to switch to the drive where
        # the temporary source file lives.
        drive = os.path.splitdrive(self.source_path)[0]
        if drive:
            os.chdir(drive)
        with open(self.source_path, 'w') as file:
            file.write('x = 123\n')

    def tearDown(self):
        shutil.rmtree(self.directory)
        if self.cwd_drive:
            os.chdir(self.cwd_drive)

    def test_absolute_path(self):
        py_compile.compile(self.source_path, self.pyc_path)
        self.assertTrue(os.path.exists(self.pyc_path))
        self.assertFalse(os.path.exists(self.cache_path))

    def test_do_not_overwrite_symlinks(self):
        # In the face of a cfile argument being a symlink, bail out.
        # Issue #17222
        try:
            os.symlink(self.pyc_path + '.actual', self.pyc_path)
        except (NotImplementedError, OSError):
            self.skipTest('need to be able to create a symlink for a file')
        else:
            assert os.path.islink(self.pyc_path)
            with self.assertRaises(FileExistsError):
                py_compile.compile(self.source_path, self.pyc_path)

    @unittest.skipIf(not os.path.exists(os.devnull) or os.path.isfile(os.devnull),
                     'requires os.devnull and for it to be a non-regular file')
    def test_do_not_overwrite_nonregular_files(self):
        # In the face of a cfile argument being a non-regular file, bail out.
        # Issue #17222
        with self.assertRaises(FileExistsError):
            py_compile.compile(self.source_path, os.devnull)

    def test_cache_path(self):
        py_compile.compile(self.source_path)
        self.assertTrue(os.path.exists(self.cache_path))

    def test_cwd(self):
        cwd = os.getcwd()
        os.chdir(self.directory)
        py_compile.compile(os.path.basename(self.source_path),
                           os.path.basename(self.pyc_path))
        os.chdir(cwd)
        self.assertTrue(os.path.exists(self.pyc_path))
        self.assertFalse(os.path.exists(self.cache_path))

    def test_relative_path(self):
        py_compile.compile(os.path.relpath(self.source_path),
                           os.path.relpath(self.pyc_path))
        self.assertTrue(os.path.exists(self.pyc_path))
        self.assertFalse(os.path.exists(self.cache_path))

    @unittest.skipIf(hasattr(os, 'geteuid') and os.geteuid() == 0,
                     'non-root user required')
    @unittest.skipIf(os.name == 'nt',
                     'cannot control directory permissions on Windows')
    def test_exceptions_propagate(self):
        # Make sure that exceptions raised thanks to issues with writing
        # bytecode.
        # http://bugs.python.org/issue17244
        mode = os.stat(self.directory)
        os.chmod(self.directory, stat.S_IREAD)
        try:
            with self.assertRaises(IOError):
                py_compile.compile(self.source_path, self.pyc_path)
        finally:
            os.chmod(self.directory, mode.st_mode)


if __name__ == "__main__":
    unittest.main()
