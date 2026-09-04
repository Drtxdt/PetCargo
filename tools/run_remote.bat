@echo off
setlocal
cd /d "%~dp0\.."
if "%~1"=="" (
  echo Usage: tools\run_remote.bat http://ROBOT_IP:8080 [COM_PORT]
  exit /b 2
)
if "%~2"=="" (
  python tools\remote_bridge.py --robot "%~1"
) else (
  python tools\remote_bridge.py --robot "%~1" --port "%~2"
)
