@rem Used by the buildbot "test" step.
cd PCbuild
call rt.bat -d -q -uall -rwW -n --timeout=3600 %1 %2 %3 %4 %5 %6 %7 %8 %9
