@echo off
set ktall=1
for /f "delims=" %%i in (nfiles) do set %%~i=a
for /r . %%i in (*.bat) do (if not defined %%~nxi call %%i)
pause
echo on

