cmd /c scons --debugbuild msvcproj --clean
cmd /c scons msvcproj --clean
del *.vcproj
del *.sln
cmd /c scons --debugbuild msvcproj
cmd /c scons msvcproj
