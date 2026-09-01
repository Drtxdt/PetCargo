@echo off
setlocal enabledelayedexpansion

rem ============================================================================
rem Mars SDK Windows Build Script - Auto-Setup Version
rem ============================================================================
rem Developer Config: Change TOOLCHAIN_ROOT to customize toolchain location
rem ============================================================================
set TOOLCHAIN_ROOT=D:\mars-toolchain

:: 获取脚本所在目录
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

:: 工具链目录（直接使用 TOOLCHAIN_ROOT）
set TOOLCHAIN_DIR=%TOOLCHAIN_ROOT%

:: ============================================================================
:: 工具下载地址配置（可根据需要修改）
:: ============================================================================
:: RISC-V GCC 工具链 (Nuclei) - 解压后直接是 gcc/ 目录
set GCC_URL=https://download.nucleisys.com/upload/files/toolchain/gcc/nuclei_riscv_newlibc_prebuilt_win32_nuclei-2024.zip
set GCC_ZIP=nuclei_riscv_newlibc_prebuilt_win32_nuclei-2024.zip

:: Python 嵌入式版本（放在单独的 python 文件夹中）
set PYTHON_NAME=python
set PYTHON_VERSION=3.12.0
set PYTHON_URL=https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip

:: CMake
set CMAKE_NAME=cmake-3.28.0-windows-x86_64
set CMAKE_URL=https://listenai-firmware-delivery.oss-cn-beijing.aliyuncs.com/Mars/toolchain/cmake-3.28.0-windows-x86_64.zip

:: Ninja (官方地址)
set NINJA_NAME=ninja-win
set NINJA_URL=https://listenai-firmware-delivery.oss-cn-beijing.aliyuncs.com/Mars/toolchain/ninja-win.zip

:: pip get-pip.py
set GETPIP_URL=https://bootstrap.pypa.io/get-pip.py

:: ============================================================================
:: 设置环境变量
:: ============================================================================
:: GCC 工具链解压后直接是 gcc/ 目录，所以 GCC_DIR 就是 TOOLCHAIN_DIR
set GCC_DIR=%TOOLCHAIN_DIR%
set PYTHON_DIR=%TOOLCHAIN_DIR%\%PYTHON_NAME%
set CMAKE_DIR=%TOOLCHAIN_DIR%\%CMAKE_NAME%
set NINJA_DIR=%TOOLCHAIN_DIR%\%NINJA_NAME%

:: 添加到 PATH
set PATH=%GCC_DIR%\gcc\bin;%CMAKE_DIR%\bin;%NINJA_DIR%;%PYTHON_DIR%;%PYTHON_DIR%\Scripts;%PATH%

:: ============================================================================
:: 默认参数
:: ============================================================================
set TARGET=
set VERBOSE=
set REMOVE=0
set SETUP_ONLY=0
set GENMAKE=Ninja
set BUILDIR=build

:: Kconfig 配置
set KCONFIG_CONFIG=.config
set KCONFIG_HEADER=include\autoconf.h

:: ============================================================================
:: 解析命令行参数
:: ============================================================================
:parse_args
if "%~1"=="" goto done_args
if /i "%~1"=="-h" goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="--setup" set SETUP_ONLY=1& shift& goto parse_args
if /i "%~1"=="-v" set VERBOSE=-v& shift& goto parse_args
if /i "%~1"=="--verbose" set VERBOSE=-v& shift& goto parse_args
if /i "%~1"=="-r" set REMOVE=1& shift& goto parse_args
if /i "%~1"=="--remove" set REMOVE=1& shift& goto parse_args
if /i "%~1"=="-R" set REMOVE=2& shift& goto parse_args
if /i "%~1"=="--Remove" set REMOVE=2& shift& goto parse_args
:: 其他参数作为 target
set TARGET=%TARGET% %~1
shift
goto parse_args

:done_args
:: 如果没有指定 target，默认为 all
if "%TARGET%"=="" set TARGET=all
:: 去除前导空格
for /f "tokens=* delims= " %%a in ("%TARGET%") do set TARGET=%%a

:: ============================================================================
:: Step 1: 检查并创建 toolchain 目录
:: ============================================================================
echo.
echo ============================================================
echo   Mars SDK Build System - Checking Environment [Windows]
echo ============================================================
echo.

if not exist "%TOOLCHAIN_DIR%" (
    echo [INFO] Creating toolchain directory...
    mkdir "%TOOLCHAIN_DIR%"
)

:: ============================================================================
:: Step 2: 检查并下载所需工具
:: ============================================================================
call :check_and_download_tools
if errorlevel 1 (
    echo [ERROR] Failed to setup toolchain
    exit /b 1
)

:: 如果只是 setup 模式，到此结束
if %SETUP_ONLY%==1 (
    echo.
    echo [INFO] Setup completed successfully
    goto :eof
)

:: 生成音频头文件
cd .\source\listen\tone
..\..\..\tools\tone_tool\ToneTool\tone_tool_header.exe ..\..\..\res\audio\
cd ..\..\..

:: ============================================================================
:: Step 3: 执行构建
:: ============================================================================

:: 删除构建目录
if %REMOVE% GEQ 1 (
    echo [BUILD] Removing [%BUILDIR%] ...
    if exist "%BUILDIR%" rmdir /s /q "%BUILDIR%"
)
if %REMOVE% GEQ 2 (
    echo [BUILD] Removing [%KCONFIG_CONFIG% %KCONFIG_HEADER%] ...
    if exist "%KCONFIG_CONFIG%" del /q "%KCONFIG_CONFIG%"
    if exist "%KCONFIG_HEADER%" del /q "%KCONFIG_HEADER%"
)

:: 生成构建系统（检查 build.ninja 而非目录，避免 configure 失败后目录残留导致跳过重新 configure）
if not exist "%BUILDIR%\build.ninja" call :do_genmake

:: 执行构建
if not "%TARGET%"=="" (
    echo [BUILD] Building [%TARGET%] ...
    cmake --build %BUILDIR% %VERBOSE% --parallel --target %TARGET%
)

:: 检查构建结果
echo.
if exist "build\bin\app.bin" (
    echo ========================================
    echo   Mars SDK Build SUCCESS
    echo ========================================
    if exist "tools\bin\csk5060-enc-pack.exe" (
        tools\bin\csk5060-enc-pack.exe build\bin\app.bin build\bin\app.symb build\bin\app.img 0 CSK3021 921600
    )
) else (
    echo ========================================
    echo   Mars SDK Build FAILED
    echo ========================================
)

goto :eof

:: ============================================================================
:: 子程序：检查并下载所需工具
:: ============================================================================
:check_and_download_tools
echo [CHECK] Checking required tools...
echo.

:: 检查 GCC 工具链（解压后直接是 gcc/ 目录）
if not exist "%GCC_DIR%\gcc\bin\riscv64-unknown-elf-gcc.exe" (
    echo [DOWNLOAD] RISC-V GCC Toolchain not found, downloading...
    call :download_and_extract "%GCC_URL%" "%GCC_ZIP%" "%TOOLCHAIN_DIR%"
    if errorlevel 1 exit /b 1
) else (
    echo [OK] RISC-V GCC Toolchain
)

:: 检查 Python
if not exist "%PYTHON_DIR%\python.exe" (
    echo [DOWNLOAD] Python not found, downloading...
    if not exist "%PYTHON_DIR%" mkdir "%PYTHON_DIR%"
    call :download_and_extract "%PYTHON_URL%" "python-%PYTHON_VERSION%-embed-amd64.zip" "%PYTHON_DIR%"
    if errorlevel 1 exit /b 1
    :: 配置 Python 以支持 pip
    call :setup_python
) else (
    echo [OK] Python
)

:: 检查 CMake
if not exist "%CMAKE_DIR%\bin\cmake.exe" (
    echo [DOWNLOAD] CMake not found, downloading...
    call :download_and_extract "%CMAKE_URL%" "%CMAKE_NAME%.zip" "%TOOLCHAIN_DIR%"
    if errorlevel 1 exit /b 1
) else (
    echo [OK] CMake
)

:: 检查 Ninja
if not exist "%NINJA_DIR%\ninja.exe" (
    echo [DOWNLOAD] Ninja not found, downloading...
    if not exist "%NINJA_DIR%" mkdir "%NINJA_DIR%"
    call :download_and_extract "%NINJA_URL%" "%NINJA_NAME%.zip" "%NINJA_DIR%"
    if errorlevel 1 exit /b 1
) else (
    echo [OK] Ninja
)

:: 检查 pyelftools
call :check_pyelftools

echo.
echo [CHECK] All tools are ready!
exit /b 0

:: ============================================================================
:: 子程序：下载并解压文件
:: ============================================================================
:download_and_extract
set DL_URL=%~1
set DL_FILE=%~2
set DL_DEST=%~3
set DL_PATH=%TOOLCHAIN_DIR%\%DL_FILE%

echo         URL: %DL_URL%
echo         Downloading...

:: 使用 PowerShell 下载
powershell -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $ProgressPreference = 'SilentlyContinue'; Invoke-WebRequest -Uri '%DL_URL%' -OutFile '%DL_PATH%' }"
if errorlevel 1 (
    echo [ERROR] Download failed
    exit /b 1
)

echo         Extracting to %DL_DEST%...
:: 使用 PowerShell 解压
powershell -Command "& { Expand-Archive -Path '%DL_PATH%' -DestinationPath '%DL_DEST%' -Force }"
if errorlevel 1 (
    echo [ERROR] Extract failed
    exit /b 1
)

:: 删除下载的 zip 文件
del /q "%DL_PATH%" 2>nul
echo         Done!
exit /b 0

:: ============================================================================
:: 子程序：配置 Python 以支持 pip
:: ============================================================================
:setup_python
echo [SETUP] Configuring Python for pip support...

:: 修改 python312._pth 文件以启用 site-packages
:: 查找 _pth 文件（支持不同 Python 版本）
for %%f in ("%PYTHON_DIR%\python*._pth") do set PTH_FILE=%%f
if exist "%PTH_FILE%" (
    for %%f in ("%PYTHON_DIR%\python*.zip") do set PY_ZIP=%%~nxf
    echo !PY_ZIP!> "%PTH_FILE%"
    echo .>> "%PTH_FILE%"
    echo import site>> "%PTH_FILE%"
)

:: 下载 get-pip.py
echo         Downloading get-pip.py...
powershell -Command "& { [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $ProgressPreference = 'SilentlyContinue'; Invoke-WebRequest -Uri '%GETPIP_URL%' -OutFile '%PYTHON_DIR%\get-pip.py' }"

:: 安装 pip
echo         Installing pip...
"%PYTHON_DIR%\python.exe" "%PYTHON_DIR%\get-pip.py" --no-warn-script-location >nul 2>&1

:: 删除 get-pip.py
del /q "%PYTHON_DIR%\get-pip.py" 2>nul
echo         Python configured!
exit /b 0

:: ============================================================================
:: 子程序：检查 pyelftools
:: ============================================================================
:check_pyelftools
"%PYTHON_DIR%\python.exe" -c "import elftools" >nul 2>&1
if errorlevel 1 (
    echo [INSTALL] Installing pyelftools...
    "%PYTHON_DIR%\python.exe" -m pip install pyelftools --no-warn-script-location -q
)
exit /b 0

:: ============================================================================
:: 子程序：生成构建系统
:: ============================================================================
:do_genmake
if not exist "%KCONFIG_CONFIG%" (
    echo [BUILD] Generating defconfig...
    "%PYTHON_DIR%\python.exe" tools\kconfpy\kconfig.py -KKconfig -Dboards/defconfig -H%KCONFIG_HEADER% -C%KCONFIG_CONFIG%
)

echo [BUILD] Running CMake [-G%GENMAKE%]...
cmake -G"%GENMAKE%" -DPython3_EXECUTABLE="%PYTHON_DIR%\python.exe" -B %BUILDIR%
exit /b 0

:: ============================================================================
:: 子程序：显示帮助
:: ============================================================================
:show_help
echo.
echo ============================================================
echo   Mars SDK Build Script - Auto Setup Version
echo ============================================================
echo.
echo Usage: %~nx0 [options] [targets]
echo.
echo Options:
echo   -h, --help      Show this help message
echo   --setup         Only download and setup toolchain, don't build
echo   -v, --verbose   Enable verbose output when building
echo   -r, --remove    Remove build directory before building
echo   -R, --Remove    Remove build directory and config files
echo.
echo Targets:
echo   all             Build all (default)
echo   clean           Clean build files
echo   help            Show available CMake targets
echo   menuconfig      Run menu configuration
echo   defconfig       Generate default configuration
echo.
echo Examples:
echo   %~nx0                   Auto-setup and build
echo   %~nx0 --setup           Only setup toolchain (no build)
echo   %~nx0 -r all            Clean and rebuild
echo   %~nx0 -R all            Full clean and rebuild
echo.
echo Toolchain Directory: %TOOLCHAIN_DIR%
echo.
goto :eof
