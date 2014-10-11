import unittest
from test import support
import os
import sys


class NoAll(RuntimeError):
    pass

class FailedImport(RuntimeError):
    pass


class AllTest(unittest.TestCase):

    def check_all(self, modname):
        names = {}
        with support.check_warnings(
            (".* (module|package)", DeprecationWarning),
            ("", ResourceWarning),
            quiet=True):
            try:
                exec("import %s" % modname, names)
            except:
                # Silent fail here seems the best route since some modules
                # may not be available or not initialize properly in all
                # environments.
                raise FailedImport(modname)
        if not hasattr(sys.modules[modname], "__all__"):
            raise NoAll(modname)
        names = {}
        with self.subTest(module=modname):
            try:
                exec("from %s import *" % modname, names)
            except Exception as e:
                # Include the module name in the exception string
                self.fail("__all__ failure in {}: {}: {}".format(
                          modname, e.__class__.__name__, e))
            if "__builtins__" in names:
                del names["__builtins__"]
            keys = set(names)
            all_list = sys.modules[modname].__all__
            all_set = set(all_list)
            self.assertCountEqual(all_set, all_list, "in module {}".format(modname))
            self.assertEqual(keys, all_set, "in module {}".format(modname))

    def walk_modules(self, basedir, modpath):
        for fn in sorted(os.listdir(basedir)):
            path = os.path.join(basedir, fn)
            if os.path.isdir(path):
                pkg_init = os.path.join(path, '__init__.py')
                if os.path.exists(pkg_init):
                    yield pkg_init, modpath + fn
                    for p, m in self.walk_modules(path, modpath + fn + "."):
                        yield p, m
                continue
            if not fn.endswith('.py') or fn == '__init__.py':
                continue
            yield path, modpath + fn[:-3]

    def test_all(self):
        # Blacklisted modules and packages
        blacklist = set([
            # Will raise a SyntaxError when compiling the exec statement
            '__future__',
        ])

        if not sys.platform.startswith('java'):
            # In case _socket fails to build, make this test fail more gracefully
            # than an AttributeError somewhere deep in CGIHTTPServer.
            import _socket

        # rlcompleter needs special consideration; it import readline which
        # initializes GNU readline which calls setlocale(LC_CTYPE, "")... :-(
        try:
            import rlcompleter
            import locale
        except ImportError:
            pass
        else:
            locale.setlocale(locale.LC_CTYPE, 'C')

        ignored = []
        failed_imports = []
        lib_dir = os.path.dirname(os.path.dirname(__file__))
        for path, modname in self.walk_modules(lib_dir, ""):
            m = modname
            blacklisted = False
            while m:
                if m in blacklist:
                    blacklisted = True
                    break
                m = m.rpartition('.')[0]
            if blacklisted:
                continue
            if support.verbose:
                print(modname)
            try:
                # This heuristic speeds up the process by removing, de facto,
                # most test modules (and avoiding the auto-executing ones).
                with open(path, "rb") as f:
                    if b"__all__" not in f.read():
                        raise NoAll(modname)
                    self.check_all(modname)
            except NoAll:
                ignored.append(modname)
            except FailedImport:
                failed_imports.append(modname)

        if support.verbose:
            print('Following modules have no __all__ and have been ignored:',
                  ignored)
            print('Following modules failed to be imported:', failed_imports)


if __name__ == "__main__":
    unittest.main()
