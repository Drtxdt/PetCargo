@echo off
setlocal EnableExtensions
cd /d "%~dp0" || exit /b 1

set "SDCC=sdcc.exe"
set "PACKIHX=packihx.exe"
where /q "%SDCC%" 2>nul
if errorlevel 1 set "SDCC=D:\SDCC\bin\sdcc.exe"
where /q "%PACKIHX%" 2>nul
if errorlevel 1 set "PACKIHX=D:\SDCC\bin\packihx.exe"

if not exist "%SDCC%" where /q "%SDCC%" 2>nul
if errorlevel 1 (
  echo [ERROR] sdcc.exe not found. Install SDCC or place it in D:\SDCC\bin.
  exit /b 1
)
if not exist "%PACKIHX%" where /q "%PACKIHX%" 2>nul
if errorlevel 1 (
  echo [ERROR] packihx.exe not found.
  exit /b 1
)

if not exist build mkdir build
set "CFLAGS=-mmcs51 --std-sdcc99 --model-large --opt-code-size --xram-size 2048 --code-size 61440 -Iinclude -DFOSC=11059200UL"
set "SRC=main.c src\hal.c src\protocol.c src\devices.c src\oled.c src\petcargo.c"
set "REL=build\main.rel build\hal.rel build\protocol.rel build\devices.rel build\oled.rel build\petcargo.rel"

for %%F in (%SRC%) do (
  echo [CC] %%F
  "%SDCC%" %CFLAGS% -c "%%F" -o "build\%%~nF.rel"
  if errorlevel 1 exit /b 1
)

echo [LD] build\petcargo.ihx
"%SDCC%" -mmcs51 --model-large --xram-size 2048 --code-size 61440 %REL% -o build\petcargo.ihx
if errorlevel 1 exit /b 1
"%PACKIHX%" build\petcargo.ihx > build\petcargo.hex
if errorlevel 1 exit /b 1
echo [OK] build\petcargo.hex
