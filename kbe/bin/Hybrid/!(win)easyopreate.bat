@echo off
set curpath=%~dp0
if defined KBE_ROOT (echo %KBE_ROOT%) else set KBE_ROOT=%curpath:~0,-15%
if defined KBE_RES_PATH (echo %KBE_RES_PATH%) else set KBE_RES_PATH=%KBE_ROOT%kbe/res/;%KBE_ROOT%demo/;%KBE_ROOT%demo/res/
if defined KBE_HYBRID_PATH (echo %KBE_HYBRID_PATH%) else set KBE_HYBRID_PATH=%KBE_ROOT%kbe/bin/Hybrid/

set uid=%random%%%32760+1

:start
@echo off
color 0A
echo. ╔═════════════════╗
echo. ║  请选择要进行的操作，然后按回车  ║
echo. ║═════════════════║
echo. ║                                  ║
echo. ║     1.启动全部服务器             ║
echo. ║                                  ║
echo. ║     2.关闭全部服务器             ║
echo. ║                                  ║
echo. ║     3.重启服务器base             ║
echo. ║                                  ║
echo. ║     4.重启服务器cell             ║
echo. ║                                  ║
echo. ║     5.重启客户端                 ║
echo. ║                                  ║
echo. ║     9.清除日志                   ║
echo. ║                                  ║
echo. ║     99.清除屏幕                  ║
echo. ║                                  ║
echo. ║     999.退出命令窗口             ║
echo. ╚═════════════════╝
echo.             
set DebugDir=%cd%\Server\Debug\
set ReleaseDir=%cd%\Server\Release\
set ClientDir=%cd%\Client\TestClient\Debug\
:cho
set choice=
set /p choice=          请选择:
IF NOT "%choice%"=="" SET choice=%choice:~0,1%
if /i "%choice%"=="99" rmdir /q /s D:\kbe\kbengine\kbe\bin\Hybrid\logs & cls
if /i "%choice%"=="99" cls & goto start
if /i "%choice%"=="999" exit
if /i "%choice%"=="1" start kbmachine.exe & start billingsystem.exe & ping 127.0.0.1 -n 1 & start dbmgr.exe & ping 127.0.0.1 -n 2 & start baseappmgr.exe & ping 127.0.0.1 -n 1 & start cellappmgr.exe & ping 127.0.0.1 -n 1 & start baseapp.exe & start baseapp.exe & ping 127.0.0.1 -n 1 & start cellapp.exe & ping 127.0.0.1 -n 1 & start loginapp.exe
if /i "%choice%"=="2" taskkill /f /t /im kbmachine.exe /im dbmgr.exe /im baseappmgr.exe /im cellappmgr.exe /im baseapp.exe /im cellapp.exe /im loginapp.exe /im billingsystem.exe /im bots.exe
if /i "%choice%"=="3" taskkill /f /t /im baseapp.exe & ping 127.0.0.1 -n 10 & start baseapp.exe
if /i "%choice%"=="4" taskkill /f /t /im cell.exe & ping 127.0.0.1 -n 10 & start cell.exe
if /i "%choice%"=="5" taskkill /f /t /im game.exe & start D:\kbe\kbengine_unity3d_demo\game.exe
echo.
goto cho