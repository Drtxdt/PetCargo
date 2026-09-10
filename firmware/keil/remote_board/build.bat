@echo off
setlocal EnableExtensions
cd /d "%~dp0" || exit /b 1
set "KEIL_BIN=%KEIL_C51_BIN%"
if not defined KEIL_BIN if exist "D:\keil\C51\BIN\C51.exe" set "KEIL_BIN=D:\keil\C51\BIN"
if not defined KEIL_BIN if exist "C:\Keil_v5\C51\BIN\C51.exe" set "KEIL_BIN=C:\Keil_v5\C51\BIN"
if not defined KEIL_BIN if exist "C:\Keil\C51\BIN\C51.exe" set "KEIL_BIN=C:\Keil\C51\BIN"
if not exist "%KEIL_BIN%\C51.exe" echo [ERROR] Set KEIL_C51_BIN to the directory containing C51.exe.& exit /b 1
if not exist "..\vendor\STCBSP_V3.6.LIB" echo [ERROR] Missing ..\vendor\STCBSP_V3.6.LIB& exit /b 1
if not exist "..\vendor\inc\sys.H" echo [ERROR] Missing BSP headers in ..\vendor\inc& exit /b 1
if not exist build_keil mkdir build_keil
del /q build_keil\main.obj build_keil\PetCargo_Remote_Keil build_keil\PetCargo_Remote_Keil.hex build_keil\PETCARGO_REMOTE_KEIL.M51 2>nul
echo [CC] PetCargo Windows remote board
"%KEIL_BIN%\C51.exe" main.c OBJECT(build_keil\main.obj) SMALL OPTIMIZE(8,SIZE) INCDIR(..\vendor\inc) PRINT(build_keil\compile.lst) > build_keil\compile.log
if errorlevel 1 type build_keil\compile.log& exit /b 1
echo [LD] PetCargo remote board + STCBSP_V3.6.LIB
"%KEIL_BIN%\BL51.exe" "build_keil\main.obj, ..\vendor\STCBSP_V3.6.LIB" TO "build_keil\PetCargo_Remote_Keil" > build_keil\link.log
if not exist build_keil\PetCargo_Remote_Keil type build_keil\link.log& exit /b 1
"%KEIL_BIN%\OH51.exe" build_keil\PetCargo_Remote_Keil > build_keil\hex.log
if not exist build_keil\PetCargo_Remote_Keil.hex type build_keil\hex.log& exit /b 1
echo [OK] %CD%\build_keil\PetCargo_Remote_Keil.hex
exit /b 0
