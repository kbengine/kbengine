@echo off
set curpath=%~dp0

cd ..
set KBE_ROOT=%cd%
set KBE_RES_PATH=%KBE_ROOT%/kbe/res/;%curpath%/;%curpath%/scripts/;%curpath%/res/
set KBE_BIN_PATH=%KBE_ROOT%/kbe/bin/server/

if defined uid (echo UID = %uid%) else set uid=%random%%%32760+1

cd %curpath%
call "kill_server.bat"

echo KBE_ROOT = %KBE_ROOT%
echo KBE_RES_PATH = %KBE_RES_PATH%
echo KBE_BIN_PATH = %KBE_BIN_PATH%

start %KBE_BIN_PATH%/machine.exe --cid=1000 --grouporder=1  --globalorder=1
start %KBE_BIN_PATH%/logger.exe --cid=2000 --grouporder=1 --globalorder=2
start %KBE_BIN_PATH%/interfaces.exe --cid=3000 --grouporder=1 --globalorder=3
start %KBE_BIN_PATH%/dbmgr.exe --cid=4000 --grouporder=1 --globalorder=4
start %KBE_BIN_PATH%/baseappmgr.exe --cid=5000 --grouporder=1  --globalorder=5
start %KBE_BIN_PATH%/cellappmgr.exe --cid=6000 --grouporder=1  --globalorder=6
start %KBE_BIN_PATH%/baseapp.exe --cid=7000 --grouporder=1  --globalorder=7
start %KBE_BIN_PATH%/cellapp.exe --cid=8000 --grouporder=1  --globalorder=8
start %KBE_BIN_PATH%/loginapp.exe --cid=9000 --grouporder=1  --globalorder=9