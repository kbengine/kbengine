#!/usr/bin/env python3
import re
import sys
import shutil
import os.path
import subprocess
import sysconfig

import reindent
import untabify


SRCDIR = sysconfig.get_config_var('srcdir')


def n_files_str(count):
    """Return 'N file(s)' with the proper plurality on 'file'."""
    return "{} file{}".format(count, "s" if count != 1 else "")


def status(message, modal=False, info=None):
    """Decorator to output status info to stdout."""
    def decorated_fxn(fxn):
        def call_fxn(*args, **kwargs):
            sys.stdout.write(message + ' ... ')
            sys.stdout.flush()
            result = fxn(*args, **kwargs)
            if not modal and not info:
                print("done")
            elif info:
                print(info(result))
            else:
                print("yes" if result else "NO")
            return result
        return call_fxn
    return decorated_fxn


def mq_patches_applied():
    """Check if there are any applied MQ patches."""
    cmd = 'hg qapplied'
    with subprocess.Popen(cmd.split(),
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE) as st:
        bstdout, _ = st.communicate()
        return st.returncode == 0 and bstdout


@status("Getting the list of files that have been added/changed",
        info=lambda x: n_files_str(len(x)))
def changed_files():
    """Get the list of changed or added files from Mercurial."""
    if not os.path.isdir(os.path.join(SRCDIR, '.hg')):
        sys.exit('need a checkout to get modified files')

    cmd = 'hg status --added --modified --no-status'
    if mq_patches_applied():
        cmd += ' --rev qparent'
    with subprocess.Popen(cmd.split(), stdout=subprocess.PIPE) as st:
        return [x.decode().rstrip() for x in st.stdout]


def report_modified_files(file_paths):
    count = len(file_paths)
    if count == 0:
        return n_files_str(count)
    else:
        lines = ["{}:".format(n_files_str(count))]
        for path in file_paths:
            lines.append("  {}".format(path))
        return "\n".join(lines)


@status("Fixing whitespace", info=report_modified_files)
def normalize_whitespace(file_paths):
    """Make sure that the whitespace for .py files have been normalized."""
    reindent.makebackup = False  # No need to create backups.
    fixed = [path for path in file_paths if path.endswith('.py') and
             reindent.check(os.path.join(SRCDIR, path))]
    return fixed


@status("Fixing C file whitespace", info=report_modified_files)
def normalize_c_whitespace(file_paths):
    """Report if any C files """
    fixed = []
    for path in file_paths:
        abspath = os.path.join(SRCDIR, path)
        with open(abspath, 'r') as f:
            if '\t' not in f.read():
                continue
        untabify.process(abspath, 8, verbose=False)
        fixed.append(path)
    return fixed


ws_re = re.compile(br'\s+(\r?\n)$')

@status("Fixing docs whitespace", info=report_modified_files)
def normalize_docs_whitespace(file_paths):
    fixed = []
    for path in file_paths:
        abspath = os.path.join(SRCDIR, path)
        try:
            with open(abspath, 'rb') as f:
                lines = f.readlines()
            new_lines = [ws_re.sub(br'\1', line) for line in lines]
            if new_lines != lines:
                shutil.copyfile(abspath, abspath + '.bak')
                with open(abspath, 'wb') as f:
                    f.writelines(new_lines)
                fixed.append(path)
        except Exception as err:
            print('Cannot fix %s: %s' % (path, err))
    return fixed


@status("Docs modified", modal=True)
def docs_modified(file_paths):
    """Report if any file in the Doc directory has been changed."""
    return bool(file_paths)


@status("Misc/ACKS updated", modal=True)
def credit_given(file_paths):
    """Check if Misc/ACKS has been changed."""
    return os.path.join('Misc', 'ACKS') in file_paths


@status("Misc/NEWS updated", modal=True)
def reported_news(file_paths):
    """Check if Misc/NEWS has been changed."""
    return os.path.join('Misc', 'NEWS') in file_paths

@status("configure regenerated", modal=True, info=str)
def regenerated_configure(file_paths):
    """Check if configure has been regenerated."""
    if 'configure.ac' in file_paths:
        return "yes" if 'configure' in file_paths else "no"
    else:
        return "not needed"

@status("pyconfig.h.in regenerated", modal=True, info=str)
def regenerated_pyconfig_h_in(file_paths):
    """Check if pyconfig.h.in has been regenerated."""
    if 'configure.ac' in file_paths:
        return "yes" if 'pyconfig.h.in' in file_paths else "no"
    else:
        return "not needed"

def main():
    file_paths = changed_files()
    python_files = [fn for fn in file_paths if fn.endswith('.py')]
    c_files = [fn for fn in file_paths if fn.endswith(('.c', '.h'))]
    doc_files = [fn for fn in file_paths if fn.startswith('Doc')]
    misc_files = {os.path.join('Misc', 'ACKS'), os.path.join('Misc', 'NEWS')}\
            & set(file_paths)
    # PEP 8 whitespace rules enforcement.
    normalize_whitespace(python_files)
    # C rules enforcement.
    normalize_c_whitespace(c_files)
    # Doc whitespace enforcement.
    normalize_docs_whitespace(doc_files)
    # Docs updated.
    docs_modified(doc_files)
    # Misc/ACKS changed.
    credit_given(misc_files)
    # Misc/NEWS changed.
    reported_news(misc_files)
    # Regenerated configure, if necessary.
    regenerated_configure(file_paths)
    # Regenerated pyconfig.h.in, if necessary.
    regenerated_pyconfig_h_in(file_paths)

    # Test suite run and passed.
    if python_files or c_files:
        end = " and check for refleaks?" if c_files else "?"
        print()
        print("Did you run the test suite" + end)


if __name__ == '__main__':
    main()
