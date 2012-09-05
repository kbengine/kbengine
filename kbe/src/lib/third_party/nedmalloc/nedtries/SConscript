import os, sys, platform
# Work around stupid cwd bug in scons
mypath=os.getcwd()
while not os.path.exists(os.path.join(mypath, "SConscript")):
    mypath=os.path.dirname(mypath)

Import("env")

# Test program
sources = [ "test.c" ]
objects = env.Object("test_c", source = sources)
testprogram_c = env.Program("test_c", source = objects)
sources = Command(os.path.join(mypath, 'test.cpp'), 'test.c', Copy(os.path.join(mypath, 'test.cpp'), os.path.join(mypath, 'test.c')))
objects = env.Object("test_cpp", source = sources)
testprogram_cpp = env.Program("test_cpp", source = objects)

# Benchmark program
sources = Command(os.path.join(mypath, 'benchmark.c'), 'benchmark.cpp', Copy(os.path.join(mypath, 'benchmark.c'), os.path.join(mypath, 'benchmark.cpp')))
objects = env.Object("benchmark_c", source = sources)
benchmarkprogram_c = env.Program("benchmark_c", source = objects)
sources = [ "benchmark.cpp" ]
objects = env.Object("benchmark_cpp", source = sources)
benchmarkprogram_cpp = env.Program("benchmark_cpp", source = objects)

Default([testprogram_c, benchmarkprogram_c, testprogram_cpp, benchmarkprogram_cpp])
