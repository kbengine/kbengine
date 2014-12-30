@echo off
set curpath=%~dp0
if defined KBE_ROOT (echo %KBE_ROOT%) else set KBE_ROOT=%curpath:~0,-28%
if defined KBE_RES_PATH (echo %KBE_RES_PATH%) else set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%assets/;%KBE_ROOT%assets/scripts/;%KBE_ROOT%assets/res/
if defined KBE_BIN_PATH (echo %KBE_BIN_PATH%) else set KBE_BIN_PATH=%KBE_ROOT%kbe/bin/server/

if defined uid (echo %uid%) else set uid=%random%%%32760+1

start guiconsole.exe