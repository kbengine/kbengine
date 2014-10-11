"""Script to compile the dependencies of _tkinter

Copyright (c) 2007 by Christian Heimes <christian@cheimes.de>

Licensed to PSF under a Contributor Agreement.
"""

import os
import sys

here = os.path.abspath(os.path.dirname(__file__))
par = os.path.pardir

TCL = "tcl8.6.1"
TK = "tk8.6.1"
TIX = "tix-8.4.3.3"

ROOT = os.path.abspath(os.path.join(here, par, par))
NMAKE = ('nmake /nologo /f %s %s %s')

def nmake(makefile, command="", **kw):
    defines = ' '.join(k+'='+str(v) for k, v in kw.items())
    cmd = NMAKE % (makefile, defines, command)
    print("\n\n"+cmd+"\n")
    if os.system(cmd) != 0:
        raise RuntimeError(cmd)

def build(platform, clean):
    if platform == "Win32":
        dest = os.path.join(ROOT, "tcltk")
        machine = "IX86"
    elif platform == "AMD64":
        dest = os.path.join(ROOT, "tcltk64")
        machine = "AMD64"
    else:
        raise ValueError(platform)

    # TCL
    tcldir = os.path.join(ROOT, TCL)
    if 1:
        os.chdir(os.path.join(tcldir, "win"))
        if clean:
            nmake("makefile.vc", "clean")
        nmake("makefile.vc", MACHINE=machine)
        nmake("makefile.vc", "install", INSTALLDIR=dest, MACHINE=machine)

    # TK
    if 1:
        os.chdir(os.path.join(ROOT, TK, "win"))
        if clean:
            nmake("makefile.vc", "clean", DEBUG=0, TCLDIR=tcldir)
        nmake("makefile.vc", DEBUG=0, MACHINE=machine, TCLDIR=tcldir)
        nmake("makefile.vc", "install", DEBUG=0, INSTALLDIR=dest, MACHINE=machine, TCLDIR=tcldir)

    # TIX
    if 1:
        # python9.mak is available at http://svn.python.org
        os.chdir(os.path.join(ROOT, TIX, "win"))
        if clean:
            nmake("python.mak", "clean")
        nmake("python.mak", MACHINE=machine, INSTALL_DIR=dest)
        nmake("python.mak", "install", MACHINE=machine, INSTALL_DIR=dest)

def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ("Win32", "AMD64"):
        print("%s Win32|AMD64" % sys.argv[0])
        sys.exit(1)

    if "-c" in sys.argv:
        clean = True
    else:
        clean = False

    build(sys.argv[1], clean)


if __name__ == '__main__':
    main()
