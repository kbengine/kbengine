import os, sys, platform

Import("env")
env=env.Clone()
architecture=env['VARIANT'][:env['VARIANT'].find('/')]
debugbuild="Debug" in env['VARIANT']
if sys.platform=="win32":
    if architecture=="x86":
        if   env.GetOption('sse')==1: env['CCFLAGS']+=[ "/arch:SSE" ]
        elif env.GetOption('sse')>=2: env['CCFLAGS']+=[ "/arch:SSE2" ]
        if   env.GetOption('sse')>=3: env['CPPDEFINES']+=[("__SSE3__", 1)]
        if   env.GetOption('sse')>=4: env['CPPDEFINES']+=[("__SSE4__", 1)]
else:
    if architecture=="x86":
        if env.GetOption('sse'):
            env['CCFLAGS']+=["-mfpmath=sse"]
            if env.GetOption('sse')>1: env['CCFLAGS']+=["-msse%s" % str(env.GetOption('sse'))]
            else: env['CCFLAGS']+=["-msse"]

# Am I building a debug or release build?
if debugbuild:
    env['CPPDEFINES']+=["DEBUG", "_DEBUG"]
else:
    env['CPPDEFINES']+=["NDEBUG"]

# Am I building for Windows or POSIX?
if sys.platform=='win32':
    env['CPPDEFINES']+=["WIN32", "_WINDOWS", "UNICODE", "_UNICODE"]
    env['CXXFLAGS']+=["/EHsc"]
    env['CCFLAGS']+=["/GF"]             # Eliminate duplicate strings
    env['CCFLAGS']+=["/Gy"]             # Seperate COMDATs
    env['CCFLAGS']+=["/Zi"]             # Program database debug info
    if debugbuild:
        env['CCFLAGS']+=["/Od", "/MTd"]
    else:
        env['CCFLAGS']+=["/O2", "/MT"]
        env['CCFLAGSFORNEDMALLOC']+=["/GL"]         # Do link time code generation
    env['LIBS']+=["psapi", "user32", "advapi32"]
    env['LINKFLAGS']+=["/DEBUG"]                # Output debug symbols
    env['LINKFLAGS']+=["/LARGEADDRESSAWARE"]    # Works past 2Gb
    env['LINKFLAGS']+=["/DYNAMICBASE"]          # Doesn't mind being randomly placed
    env['LINKFLAGS']+=["/NXCOMPAT"]             # Likes no execute
    env['LINKFLAGS']+=["/OPT:REF"]              # Seems to puke on load on WinXP without
    env['LINKFLAGS']+=["/MANIFEST"]             # Be UAC compatible
    env['LINKFLAGSEXE']=env['LINKFLAGS'][:]
    if env.GetOption('adminuac'): env['LINKFLAGSEXE']+=["/MANIFESTUAC:level='requireAdministrator'"]


    env['LINKFLAGS']+=["/ENTRY:DllPreMainCRTStartup"]
    env['LINKFLAGS']+=["/VERSION:1.10.0"]        # Version

    if not debugbuild:
        env['LINKFLAGS']+=["/OPT:ICF"]  # Eliminate redundants
        if env.GetOption('pgo') or os.path.exists('${VARIANT}/'+env['NEDMALLOCLIBRARYNAME']+".pgd"):
            env['LINKFLAGS']+=["/PGD:${VARIANT}/"+env['LIBRARYNAME']+".pgd"]
            if env.GetOption('pgo'):
                env['LINKFLAGS']+=["/LTCG:PGINSTRUMENT"]
            else:
                env['LINKFLAGS']+=["/LTCG:PGUPDATE"]
else:
    env['CPPDEFINES']+=[]
    env['CCFLAGS']+=["-fstrict-aliasing", "-fargument-noalias", "-Wstrict-aliasing"]
    env['CCFLAGS']+=["-Wall", "-Wno-unused"]
    if debugbuild:
        env['CCFLAGS']+=["-O0", "-g"]
    else:
        env['CCFLAGS']+=["-O2", "-g"]
    if env.GetOption('uselocks'):
        env['LIBS']+=["pthread"]
    env['LINKFLAGS']+=[]
    env['LINKFLAGSEXE']=env['LINKFLAGS'][:]

outputs={}

# Build the nedmalloc DLL
sources = ["nedmalloc.c"]
libobjects = env.SharedObject("nedmalloc.c", CPPDEFINES=env['CPPDEFINES']+["NEDMALLOC_DLL_EXPORTS"], CCFLAGS=env['CCFLAGS']+env['CCFLAGSFORNEDMALLOC'])
if sys.platform=='win32':
    if not env.GetOption("static"): libobjects+=env.SharedObject("winpatcher_nedmalloc", "winpatcher.c", CPPDEFINES=env['CPPDEFINES']+["NEDMALLOC_DLL_EXPORTS"])
    libobjects+=env.RES("nedmalloc.res", "nedmalloc_dll.rc")
    sources+=["winpatcher.c", "nedmalloc_dll.rc"]
if env.GetOption("static"):
    nedmalloclib = env.StaticLibrary(env['NEDMALLOCLIBRARYNAME'], source = libobjects)
    nedmallocliblib = nedmalloclib
else:
    nedmalloclib = env.SharedLibrary(env['NEDMALLOCLIBRARYNAME'], source = libobjects)
    if sys.platform=='win32': env.AddPostAction(nedmalloclib, 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;2')
    nedmallocliblib = nedmalloclib
    if sys.platform=='win32':
        #env.AddPreAction(env.AddPreAction(nedmalloclib, "pgomgr /clear ${VARIANT}/nedmalloc.pgd"), "pgomgr /merge ${VARIANT}/nedmalloc.pgd")
        nedmallocliblib=nedmalloclib[1]
outputs['nedmalloclib']=(nedmalloclib, sources)

if True and sys.platform=='win32':
    # Build the UMPA DLL
    if not env.GetOption("static"): libobjects=env.SharedObject("winpatcher_umpa", "winpatcher.c", CPPDEFINES=env['CPPDEFINES']+["USERMODEPAGEALLOCATOR_DLL_EXPORTS"], CCFLAGS=env['CCFLAGS']+env['CCFLAGSFORNEDMALLOC'])
    libobjects+=env.RES("nedmalloc.res", "nedmalloc_dll.rc")
    if env.GetOption("static"):
        umpalib = env.StaticLibrary(env['UMPALIBRARYNAME'], source = libobjects)
    else:
        umpalib = env.SharedLibrary(env['UMPALIBRARYNAME'], source = libobjects)
        env.AddPostAction(umpalib, 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;2')
    outputs['umpalib']=(umpalib, ["nedmalloc.c", "winpatcher.c", "nedmalloc_dll.rc"])

# C test program
sources = [ "test.c" ]
objects = env.Object("test_c", source = sources) # + [nedmallocliblib]
testlibs=[nedmallocliblib]
testprogram_c = env.Program("test_c", source = objects, LINKFLAGS=env['LINKFLAGSEXE'], LIBS = env['LIBS'] + testlibs)
outputs['testprogram_c']=(testprogram_c, sources)

# C++ test program
sources = [ "test.cpp" ]
objects = env.Object("test_cpp", source = sources) # + [nedmallocliblib]
testlibs=[nedmallocliblib]
testprogram_cpp = env.Program("test_cpp", source = objects, LINKFLAGS=env['LINKFLAGSEXE'], LIBS = env['LIBS'] + testlibs)
outputs['testprogram_cpp']=(testprogram_cpp, sources)

# PGO program
sources = [ "make_pgos.c" ]
objects = env.Object(source = sources) # + [nedmallocliblib]
testlibs=[nedmallocliblib]
make_pgos = env.Program("make_pgos", source = objects, LINKFLAGS=env['LINKFLAGSEXE'], LIBS = env['LIBS'] + testlibs)
outputs['make_pgos']=(make_pgos, sources)

# Scaling program
sources = [ "scalingtest.cpp" ]
objects = env.Object(source = sources) # + [nedmallocliblib]
scalingtest = env.Program("scalingtest", source = objects, LINKFLAGS=env['LINKFLAGSEXE'])
outputs['scalingtest']=(scalingtest, sources)

Return("outputs")
