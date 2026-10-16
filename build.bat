@echo off
setlocal enabledelayedexpansion

rem Windows build script (counterpart of build.sh).
rem Usage: build.bat [--clean] [--config <CMAKE_ARG> ...]
rem   --clean   Remove the build directory first.
rem Extra arguments are passed through to CMake.

set "BUILD_DIR=build-win"
set "CLEAN=0"
set "EXTRA_CMAKE="

:parse
if "%~1"=="" goto :run
if /i "%~1"=="--clean" (
    set "CLEAN=1"
    shift
    goto :parse
)
if /i "%~1"=="--help" (
    echo Usage: build.bat [--clean] [--config ^<CMAKE_ARG^> ...]
    echo   --clean   Remove the build directory first.
    echo Extra arguments are passed through to CMake.
    exit /b 0
)
set "EXTRA_CMAKE=!EXTRA_CMAKE! %~1"
shift
goto :parse

:run
if "%CLEAN%"=="1" (
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    echo Cleaned %BUILD_DIR%
)
rem Prefer Visual Studio solution generation; fall back to the default
rem generator when the VS generator is unavailable.
cmake -S . -B "%BUILD_DIR%" -G "Visual Studio 18 2026" -A x64 %EXTRA_CMAKE%
if errorlevel 1 (
    echo VS generator unavailable, falling back to default generator...
    cmake -S . -B "%BUILD_DIR%" %EXTRA_CMAKE%
    if errorlevel 1 exit /b 1
)

cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 exit /b 1

echo.
echo [ Running tests ]
"%BUILD_DIR%\tests\Release\test_cases.exe"
exit /b %errorlevel%
