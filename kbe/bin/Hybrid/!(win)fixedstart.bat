@echo off
set curpath=%~dp0
if defined KBE_ROOT (echo %KBE_ROOT%) else set KBE_ROOT=%curpath:~0,-15%
if defined KBE_RES_PATH (echo %KBE_RES_PATH%) else set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/res/
if defined KBE_HYBRID_PATH (echo %KBE_HYBRID_PATH%) else set KBE_HYBRID_PATH=%KBE_ROOT%kbe/bin/Hybrid/

if defined uid (echo %uid%) else set uid=%random%%%32760+1

call "!(win)kill.bat"

start kbmachine.exe --cid=2129652375332859700 --grouporder=1  --globalorder=1
rem start messagelog.exe --cid=1129653375331859700 --grouporder=1 --globalorder=2
start billingsystem.exe --cid=1129652375332859700 --grouporder=1 --globalorder=3
start dbmgr.exe --cid=3129652375332859700 --grouporder=1 --globalorder=4
start baseappmgr.exe --cid=4129652375332859700 --grouporder=1  --globalorder=5
start cellappmgr.exe --cid=5129652375332859700 --grouporder=1  --globalorder=6
start baseapp.exe --cid=6129652375332859700 --grouporder=1  --globalorder=7
start cellapp.exe --cid=7129652375332859700 --grouporder=1  --globalorder=8
start loginapp.exe --cid=8129652375332859700 --grouporder=1  --globalorder=9