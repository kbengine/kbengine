"""
Virtual environment (venv) package for Python. Based on PEP 405.

Copyright (C) 2011-2014 Vinay Sajip.
Licensed to the PSF under a contributor agreement.

usage: python -m venv [-h] [--system-site-packages] [--symlinks] [--clear]
            [--upgrade]
            ENV_DIR [ENV_DIR ...]

Creates virtual Python environments in one or more target directories.

positional arguments:
  ENV_DIR               A directory to create the environment in.

optional arguments:
  -h, --help            show this help message and exit
  --system-site-packages
                        Give the virtual environment access to the system
                        site-packages dir.
  --symlinks            Attempt to symlink rather than copy.
  --clear               Delete the environment directory if it already exists.
                        If not specified and the directory exists, an error is
                        raised.
  --upgrade             Upgrade the environment directory to use this version
                        of Python, assuming Python has been upgraded in-place.
  --without-pip         Skips installing or upgrading pip in the virtual
                        environment (pip is bootstrapped by default)
"""
import logging
import os
import shutil
import struct
import subprocess
import sys
import types

logger = logging.getLogger(__name__)


class EnvBuilder:
    """
    This class exists to allow virtual environment creation to be
    customized. The constructor parameters determine the builder's
    behaviour when called upon to create a virtual environment.

    By default, the builder makes the system (global) site-packages dir
    *un*available to the created environment.

    If invoked using the Python -m option, the default is to use copying
    on Windows platforms but symlinks elsewhere. If instantiated some
    other way, the default is to *not* use symlinks.

    :param system_site_packages: If True, the system (global) site-packages
                                 dir is available to created environments.
    :param clear: If True and the target directory exists, it is deleted.
                  Otherwise, if the target directory exists, an error is
                  raised.
    :param symlinks: If True, attempt to symlink rather than copy files into
                     virtual environment.
    :param upgrade: If True, upgrade an existing virtual environment.
    :param with_pip: If True, ensure pip is installed in the virtual
                     environment
    """

    def __init__(self, system_site_packages=False, clear=False,
                 symlinks=False, upgrade=False, with_pip=False):
        self.system_site_packages = system_site_packages
        self.clear = clear
        self.symlinks = symlinks
        self.upgrade = upgrade
        self.with_pip = with_pip

    def create(self, env_dir):
        """
        Create a virtual environment in a directory.

        :param env_dir: The target directory to create an environment in.

        """
        env_dir = os.path.abspath(env_dir)
        context = self.ensure_directories(env_dir)
        self.create_configuration(context)
        self.setup_python(context)
        if self.with_pip:
            self._setup_pip(context)
        if not self.upgrade:
            self.setup_scripts(context)
            self.post_setup(context)

    def clear_directory(self, path):
        for fn in os.listdir(path):
            fn = os.path.join(path, fn)
            if os.path.islink(fn) or os.path.isfile(fn):
                os.remove(fn)
            elif os.path.isdir(fn):
                shutil.rmtree(fn)

    def ensure_directories(self, env_dir):
        """
        Create the directories for the environment.

        Returns a context object which holds paths in the environment,
        for use by subsequent logic.
        """

        def create_if_needed(d):
            if not os.path.exists(d):
                os.makedirs(d)
            elif os.path.islink(d) or os.path.isfile(d):
                raise ValueError('Unable to create directory %r' % d)

        if os.path.exists(env_dir) and self.clear:
            self.clear_directory(env_dir)
        context = types.SimpleNamespace()
        context.env_dir = env_dir
        context.env_name = os.path.split(env_dir)[1]
        context.prompt = '(%s) ' % context.env_name
        create_if_needed(env_dir)
        env = os.environ
        if sys.platform == 'darwin' and '__PYVENV_LAUNCHER__' in env:
            executable = os.environ['__PYVENV_LAUNCHER__']
        else:
            executable = sys.executable
        dirname, exename = os.path.split(os.path.abspath(executable))
        context.executable = executable
        context.python_dir = dirname
        context.python_exe = exename
        if sys.platform == 'win32':
            binname = 'Scripts'
            incpath = 'Include'
            libpath = os.path.join(env_dir, 'Lib', 'site-packages')
        else:
            binname = 'bin'
            incpath = 'include'
            libpath = os.path.join(env_dir, 'lib',
                                   'python%d.%d' % sys.version_info[:2],
                                   'site-packages')
        context.inc_path = path = os.path.join(env_dir, incpath)
        create_if_needed(path)
        create_if_needed(libpath)
        # Issue 21197: create lib64 as a symlink to lib on 64-bit non-OS X POSIX
        if ((struct.calcsize('P') == 8) and (os.name == 'posix') and
            (sys.platform != 'darwin')):
            p = os.path.join(env_dir, 'lib')
            link_path = os.path.join(env_dir, 'lib64')
            os.symlink(p, link_path)
        context.bin_path = binpath = os.path.join(env_dir, binname)
        context.bin_name = binname
        context.env_exe = os.path.join(binpath, exename)
        create_if_needed(binpath)
        return context

    def create_configuration(self, context):
        """
        Create a configuration file indicating where the environment's Python
        was copied from, and whether the system site-packages should be made
        available in the environment.

        :param context: The information for the environment creation request
                        being processed.
        """
        context.cfg_path = path = os.path.join(context.env_dir, 'pyvenv.cfg')
        with open(path, 'w', encoding='utf-8') as f:
            f.write('home = %s\n' % context.python_dir)
            if self.system_site_packages:
                incl = 'true'
            else:
                incl = 'false'
            f.write('include-system-site-packages = %s\n' % incl)
            f.write('version = %d.%d.%d\n' % sys.version_info[:3])

    if os.name == 'nt':
        def include_binary(self, f):
            if f.endswith(('.pyd', '.dll')):
                result = True
            else:
                result = f.startswith('python') and f.endswith('.exe')
            return result

    def symlink_or_copy(self, src, dst):
        """
        Try symlinking a file, and if that fails, fall back to copying.
        """
        force_copy = not self.symlinks
        if not force_copy:
            try:
                if not os.path.islink(dst): # can't link to itself!
                    os.symlink(src, dst)
            except Exception:   # may need to use a more specific exception
                logger.warning('Unable to symlink %r to %r', src, dst)
                force_copy = True
        if force_copy:
            shutil.copyfile(src, dst)

    def setup_python(self, context):
        """
        Set up a Python executable in the environment.

        :param context: The information for the environment creation request
                        being processed.
        """
        binpath = context.bin_path
        exename = context.python_exe
        path = context.env_exe
        copier = self.symlink_or_copy
        copier(context.executable, path)
        dirname = context.python_dir
        if os.name != 'nt':
            if not os.path.islink(path):
                os.chmod(path, 0o755)
            for suffix in ('python', 'python3'):
                path = os.path.join(binpath, suffix)
                if not os.path.exists(path):
                    os.symlink(exename, path)
        else:
            subdir = 'DLLs'
            include = self.include_binary
            files = [f for f in os.listdir(dirname) if include(f)]
            for f in files:
                src = os.path.join(dirname, f)
                dst = os.path.join(binpath, f)
                if dst != context.env_exe:  # already done, above
                    copier(src, dst)
            dirname = os.path.join(dirname, subdir)
            if os.path.isdir(dirname):
                files = [f for f in os.listdir(dirname) if include(f)]
                for f in files:
                    src = os.path.join(dirname, f)
                    dst = os.path.join(binpath, f)
                    copier(src, dst)
            # copy init.tcl over
            for root, dirs, files in os.walk(context.python_dir):
                if 'init.tcl' in files:
                    tcldir = os.path.basename(root)
                    tcldir = os.path.join(context.env_dir, 'Lib', tcldir)
                    os.makedirs(tcldir)
                    src = os.path.join(root, 'init.tcl')
                    dst = os.path.join(tcldir, 'init.tcl')
                    shutil.copyfile(src, dst)
                    break

    def _setup_pip(self, context):
        """Installs or upgrades pip in a virtual environment"""
        # We run ensurepip in isolated mode to avoid side effects from
        # environment vars, the current directory and anything else
        # intended for the global Python environment
        cmd = [context.env_exe, '-Im', 'ensurepip', '--upgrade',
                                                    '--default-pip']
        subprocess.check_output(cmd, stderr=subprocess.STDOUT)

    def setup_scripts(self, context):
        """
        Set up scripts into the created environment from a directory.

        This method installs the default scripts into the environment
        being created. You can prevent the default installation by overriding
        this method if you really need to, or if you need to specify
        a different location for the scripts to install. By default, the
        'scripts' directory in the venv package is used as the source of
        scripts to install.
        """
        path = os.path.abspath(os.path.dirname(__file__))
        path = os.path.join(path, 'scripts')
        self.install_scripts(context, path)

    def post_setup(self, context):
        """
        Hook for post-setup modification of the venv. Subclasses may install
        additional packages or scripts here, add activation shell scripts, etc.

        :param context: The information for the environment creation request
                        being processed.
        """
        pass

    def replace_variables(self, text, context):
        """
        Replace variable placeholders in script text with context-specific
        variables.

        Return the text passed in , but with variables replaced.

        :param text: The text in which to replace placeholder variables.
        :param context: The information for the environment creation request
                        being processed.
        """
        text = text.replace('__VENV_DIR__', context.env_dir)
        text = text.replace('__VENV_NAME__', context.env_name)
        text = text.replace('__VENV_PROMPT__', context.prompt)
        text = text.replace('__VENV_BIN_NAME__', context.bin_name)
        text = text.replace('__VENV_PYTHON__', context.env_exe)
        return text

    def install_scripts(self, context, path):
        """
        Install scripts into the created environment from a directory.

        :param context: The information for the environment creation request
                        being processed.
        :param path:    Absolute pathname of a directory containing script.
                        Scripts in the 'common' subdirectory of this directory,
                        and those in the directory named for the platform
                        being run on, are installed in the created environment.
                        Placeholder variables are replaced with environment-
                        specific values.
        """
        binpath = context.bin_path
        plen = len(path)
        for root, dirs, files in os.walk(path):
            if root == path: # at top-level, remove irrelevant dirs
                for d in dirs[:]:
                    if d not in ('common', os.name):
                        dirs.remove(d)
                continue # ignore files in top level
            for f in files:
                srcfile = os.path.join(root, f)
                suffix = root[plen:].split(os.sep)[2:]
                if not suffix:
                    dstdir = binpath
                else:
                    dstdir = os.path.join(binpath, *suffix)
                if not os.path.exists(dstdir):
                    os.makedirs(dstdir)
                dstfile = os.path.join(dstdir, f)
                with open(srcfile, 'rb') as f:
                    data = f.read()
                if srcfile.endswith('.exe'):
                    mode = 'wb'
                else:
                    mode = 'w'
                    try:
                        data = data.decode('utf-8')
                        data = self.replace_variables(data, context)
                    except UnicodeDecodeError as e:
                        data = None
                        logger.warning('unable to copy script %r, '
                                       'may be binary: %s', srcfile, e)
                if data is not None:
                    with open(dstfile, mode) as f:
                        f.write(data)
                    shutil.copymode(srcfile, dstfile)


def create(env_dir, system_site_packages=False, clear=False,
                    symlinks=False, with_pip=False):
    """
    Create a virtual environment in a directory.

    By default, makes the system (global) site-packages dir *un*available to
    the created environment, and uses copying rather than symlinking for files
    obtained from the source Python installation.

    :param env_dir: The target directory to create an environment in.
    :param system_site_packages: If True, the system (global) site-packages
                                 dir is available to the environment.
    :param clear: If True and the target directory exists, it is deleted.
                  Otherwise, if the target directory exists, an error is
                  raised.
    :param symlinks: If True, attempt to symlink rather than copy files into
                     virtual environment.
    :param with_pip: If True, ensure pip is installed in the virtual
                     environment
    """
    builder = EnvBuilder(system_site_packages=system_site_packages,
                         clear=clear, symlinks=symlinks, with_pip=with_pip)
    builder.create(env_dir)

def main(args=None):
    compatible = True
    if sys.version_info < (3, 3):
        compatible = False
    elif not hasattr(sys, 'base_prefix'):
        compatible = False
    if not compatible:
        raise ValueError('This script is only for use with Python >= 3.3')
    else:
        import argparse

        parser = argparse.ArgumentParser(prog=__name__,
                                         description='Creates virtual Python '
                                                     'environments in one or '
                                                     'more target '
                                                     'directories.',
                                         epilog='Once an environment has been '
                                                'created, you may wish to '
                                                'activate it, e.g. by '
                                                'sourcing an activate script '
                                                'in its bin directory.')
        parser.add_argument('dirs', metavar='ENV_DIR', nargs='+',
                            help='A directory to create the environment in.')
        parser.add_argument('--system-site-packages', default=False,
                            action='store_true', dest='system_site',
                            help='Give the virtual environment access to the '
                                 'system site-packages dir.')
        if os.name == 'nt':
            use_symlinks = False
        else:
            use_symlinks = True
        group = parser.add_mutually_exclusive_group()
        group.add_argument('--symlinks', default=use_symlinks,
                           action='store_true', dest='symlinks',
                           help='Try to use symlinks rather than copies, '
                                'when symlinks are not the default for '
                                'the platform.')
        group.add_argument('--copies', default=not use_symlinks,
                           action='store_false', dest='symlinks',
                           help='Try to use copies rather than symlinks, '
                                'even when symlinks are the default for '
                                'the platform.')
        parser.add_argument('--clear', default=False, action='store_true',
                            dest='clear', help='Delete the contents of the '
                                               'environment directory if it '
                                               'already exists, before '
                                               'environment creation.')
        parser.add_argument('--upgrade', default=False, action='store_true',
                            dest='upgrade', help='Upgrade the environment '
                                               'directory to use this version '
                                               'of Python, assuming Python '
                                               'has been upgraded in-place.')
        parser.add_argument('--without-pip', dest='with_pip',
                            default=True, action='store_false',
                            help='Skips installing or upgrading pip in the '
                                 'virtual environment (pip is bootstrapped '
                                 'by default)')
        options = parser.parse_args(args)
        if options.upgrade and options.clear:
            raise ValueError('you cannot supply --upgrade and --clear together.')
        builder = EnvBuilder(system_site_packages=options.system_site,
                             clear=options.clear,
                             symlinks=options.symlinks,
                             upgrade=options.upgrade,
                             with_pip=options.with_pip)
        for d in options.dirs:
            builder.create(d)

if __name__ == '__main__':
    rc = 1
    try:
        main()
        rc = 0
    except Exception as e:
        print('Error: %s' % e, file=sys.stderr)
    sys.exit(rc)
