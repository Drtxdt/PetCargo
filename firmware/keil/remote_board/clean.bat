@echo off
setlocal
cd /d "%~dp0"
if exist build_keil rmdir /s /q build_keil
