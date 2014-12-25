@echo off
set curpath=%~dp0

set KBE_ROOT=%curpath:~0,-5%
set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/scripts/;%KBE_ROOT%demo/res/
set KBE_BIN_PATH=%KBE_ROOT%kbe/bin/server/

if defined uid (echo %uid%) else set uid=%random%%%32760+1

echo KBE_ROOT = %KBE_ROOT%
echo KBE_RES_PATH = %KBE_RES_PATH%
echo KBE_BIN_PATH = %KBE_BIN_PATH%


start bots.exe