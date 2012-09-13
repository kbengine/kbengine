@echo off
set curpath=%~dp0
set KBE_ROOT=%curpath:~0,-15%
set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/res/
set KBE_HYBRID_PATH=%KBE_ROOT%kbe/bin/Hybrid/

start kbmachine.exe
ping 127.0.0.1 -n 1
start messagelog.exe
ping 127.0.0.1 -n 1
start dbmgr.exe
ping 127.0.0.1 -n 2
start baseappmgr.exe
ping 127.0.0.1 -n 1
start cellappmgr.exe
ping 127.0.0.1 -n 1
start baseapp.exe
ping 127.0.0.1 -n 1
start cellapp.exe
ping 127.0.0.1 -n 1
start loginapp.exe