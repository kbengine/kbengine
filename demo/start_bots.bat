@echo off
set curpath=%~dp0

cd ..
echo %cd%
set KBE_ROOT=%cd%
set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/scripts/;%KBE_ROOT%demo/res/
set KBE_BIN_PATH=%KBE_ROOT%kbe/bin/server/

if defined uid (echo %uid%) else set uid=%random%%%32760+1

echo KBE_ROOT = %KBE_ROOT%
echo KBE_RES_PATH = %KBE_RES_PATH%
echo KBE_BIN_PATH = %KBE_BIN_PATH%

cd %curpath%
start %KBE_BIN_PATH%/bots.exe