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
set "NAME=petcargo"
set "SRC=main.c src\hal.c src\protocol.c src\devices.c src\oled.c src\petcargo.c src\runtime.c src\music.c src\score.c"
set "REL=build\main.rel build\hal.rel build\protocol.rel build\devices.rel build\oled.rel build\petcargo.rel build\runtime.rel build\music.rel build\score.rel"
if /i "%~1"=="diagnostic" (
  set "NAME=petcargo_diagnostic"
  set "SRC=diagnostic.c src\hal.c src\oled.c src\runtime.c src\music.c src\score.c"
  set "REL=build\diagnostic.rel build\hal.rel build\oled.rel build\runtime.rel build\music.rel build\score.rel"
)

for %%F in (%SRC%) do (
  echo [CC] %%F
  "%SDCC%" %CFLAGS% -c "%%F" -o "build\%%~nF.rel"
  if errorlevel 1 (
    echo [ERROR] Compilation failed: %%F
    exit /b 1
  )
)

echo [LD] build\%NAME%.ihx
"%SDCC%" -mmcs51 --model-large --xram-size 2048 --code-size 61440 %REL% -o build\%NAME%.ihx
if errorlevel 1 (
  echo [ERROR] Linking failed.
  exit /b 1
)
"%PACKIHX%" build\%NAME%.ihx > build\%NAME%.hex
if errorlevel 1 (
  echo [ERROR] HEX conversion failed.
  exit /b 1
)
echo [OK] build\%NAME%.hex
