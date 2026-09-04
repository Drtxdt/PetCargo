@echo off
setlocal EnableExtensions
cd /d "%~dp0" || exit /b 1

set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=all"

if /i "%TARGET%"=="clean" goto clean
if /i "%TARGET%"=="main" goto main
if /i "%TARGET%"=="remote" goto remote
if /i "%TARGET%"=="diagnostic" goto diagnostic
if /i "%TARGET%"=="all" goto all

echo Usage: build.bat [all^|main^|remote^|diagnostic^|clean]
exit /b 2

:all
call :build_main
if errorlevel 1 exit /b 1
call :build_diagnostic
if errorlevel 1 exit /b 1
call :build_remote
if errorlevel 1 exit /b 1
goto success

:main
call :build_main
if errorlevel 1 exit /b 1
goto success

:remote
call :build_remote
if errorlevel 1 exit /b 1
goto success

:diagnostic
call :build_diagnostic
if errorlevel 1 exit /b 1
goto success

:build_diagnostic
call "%~dp0firmware\stc\build.bat" diagnostic
exit /b %errorlevel%

:build_main
echo.
echo ============================================================
echo [MAIN] Compile every C file separately, then link PetCargo
echo ============================================================
call "%~dp0firmware\stc\build.bat"
if errorlevel 1 (
  echo [ERROR] Main-board firmware build failed.
  exit /b 1
)
exit /b 0

:build_remote
echo.
echo ============================================================
echo [REMOTE] Compile every C file separately, then link remote
echo ============================================================
call "%~dp0firmware\stc_remote\build.bat"
if errorlevel 1 (
  echo [ERROR] Remote firmware build failed.
  exit /b 1
)
exit /b 0

:clean
for %%D in ("firmware\stc\build" "firmware\stc_remote\build") do (
  if exist "%%~D" (
    echo [CLEAN] %%~D
    rmdir /s /q "%%~D"
    if errorlevel 1 exit /b 1
  )
)
echo [OK] Firmware build directories removed.
exit /b 0

:success
cd /d "%~dp0"
echo.
echo ============================================================
echo [OK] Requested firmware build completed successfully.
echo ============================================================
if exist firmware\stc\build\petcargo.hex echo   firmware\stc\build\petcargo.hex
if exist firmware\stc\build\petcargo_diagnostic.hex echo   firmware\stc\build\petcargo_diagnostic.hex
if exist firmware\stc_remote\build\petcargo_remote.hex echo   firmware\stc_remote\build\petcargo_remote.hex
exit /b 0
