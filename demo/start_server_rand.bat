@echo off
set curpath=%~dp0

cd ..
set KBE_ROOT=%cd%
set KBE_RES_PATH=%KBE_ROOT%/kbe/res/;%curpath%/;%curpath%/scripts/;%curpath%/res/
set KBE_BIN_PATH=%KBE_ROOT%/kbe/bin/server/

set uid=%random%%%32760+1

cd %curpath%
call "kill_server.bat"

echo KBE_ROOT = %KBE_ROOT%
echo KBE_RES_PATH = %KBE_RES_PATH%
echo KBE_BIN_PATH = %KBE_BIN_PATH%

start %KBE_BIN_PATH%/machine.exe
start %KBE_BIN_PATH%/interfaces.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/logger.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/dbmgr.exe
ping 127.0.0.1 -n 2
start %KBE_BIN_PATH%/baseappmgr.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/cellappmgr.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/baseapp.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/cellapp.exe
ping 127.0.0.1 -n 1
start %KBE_BIN_PATH%/loginapp.exe