@echo off
setlocal EnableExtensions
cd /d "%~dp0" || exit /b 1
set "SDCC=sdcc.exe"
set "PACKIHX=packihx.exe"
where /q "%SDCC%" 2>nul
if errorlevel 1 set "SDCC=D:\SDCC\bin\sdcc.exe"
where /q "%PACKIHX%" 2>nul
if errorlevel 1 set "PACKIHX=D:\SDCC\bin\packihx.exe"
if not exist build mkdir build
"%SDCC%" -mmcs51 --std-sdcc99 --model-large --opt-code-size --xram-size 2048 --code-size 61440 -Iinclude main.c -o build\petcargo_remote.ihx
if errorlevel 1 exit /b 1
"%PACKIHX%" build\petcargo_remote.ihx > build\petcargo_remote.hex
if errorlevel 1 exit /b 1
echo [OK] build\petcargo_remote.hex
