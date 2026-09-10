@echo off
setlocal EnableExtensions
cd /d "%~dp0" || exit /b 1
set "TARGET=%~1"
if not defined TARGET set "TARGET=all"
if /I "%TARGET%"=="main" goto main
if /I "%TARGET%"=="remote" goto remote
if /I not "%TARGET%"=="all" echo Usage: build_keil.bat [all^|main^|remote]& exit /b 2
call firmware\keil\main_board\build.bat || exit /b 1
call firmware\keil\remote_board\build.bat || exit /b 1
echo [OK] All Keil/BSP targets built. Existing SDCC build directories were not touched.
exit /b 0
:main
call firmware\keil\main_board\build.bat
exit /b %errorlevel%
:remote
call firmware\keil\remote_board\build.bat
exit /b %errorlevel%
