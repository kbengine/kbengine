@echo off
set curpath=%~dp0
set KBE_ROOT=%curpath:~0,-15%
set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/res/
set KBE_HYBRID_PATH=%KBE_ROOT%kbe/bin/Hybrid/

start client_d