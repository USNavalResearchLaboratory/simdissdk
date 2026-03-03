@echo off

:: Windows bootstrap script for developers.
:: Inspired by a similar script from the osgEarth project (https://github.com/gwaldron/osgearth).
::
:: This script simplifies the initial build configuration process on Windows. It is designed to be run
:: from the root of the project repository.
::
:: The script performs the following actions:
:: 1. Locates the vcpkg installation on the system.
:: 2. Sets up a build directory.
:: 3. Runs CMake to configure the project, pointing it to the vcpkg manifest located at
::    <source_dir>/vcpkg/ports/simdissdk/vcpkg.json. This triggers vcpkg to automatically
::    download and build all required dependencies.
::
:: The result is a ready-to-use Visual Studio solution in the specified build directory.
:: The script targets Visual Studio 18 2026; edit the `options` for to change the default.
::
:: Usage: See the :usage section at the end of this file for details.

setlocal
set ERROR_MSG=

if "%VCPKG_ROOT%" == "" (
    FOR /F "tokens=*" %%X IN ('where vcpkg.exe') do (SET VCPKG_ROOT=%%~dpX)
)

:: Verify vcpkg is available
set VCPKG_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

if not exist %VCPKG_TOOLCHAIN_FILE% (
    set ERROR_MSG=Cannot find vcpkg. Please ensure vcpkg.exe is in your PATH, or set the VCPKG_ROOT environment variable to your vcpkg install location
    goto :usage
)

:: Argument parser from: https://stackoverflow.com/a/8162578
setlocal enableDelayedExpansion
set "options=-S:. -B:..\build -I:..\install -G:"Visual Studio 18 2026" -A:x64"
for %%O in (%options%) do for /f "tokens=1,* delims=:" %%A in ("%%O") do set "%%A=%%~B"
:loopArgs
    if not "%~1"=="" (
      set "test=!options:*%~1:=! "
      if "!test!"=="!options! " (
          echo Error: Invalid option %~1
      ) else if "!test:~0,1!"==" " (
          set "%~1=1"
      ) else (
          setlocal disableDelayedExpansion
          set "val=%~2"
          call :escapeVal
          setlocal enableDelayedExpansion
          for /f delims^=^ eol^= %%A in ("!val!") do endlocal&endlocal&set "%~1=%%A" !
          shift /1
      )
      shift /1
      goto :loopArgs
    )
    goto :endArgs
:escapeVal
    set "val=%val:^=^^%"
    set "val=%val:!=^!%"
    exit /b
:endArgs

set COMPILER=!-G!
set ARCHITECTURE=!-A!

:: Expand to absolute paths
call :realpath !-S!
set SOURCE_DIR=%RETVAL%
call :realpath !-B!
set BUILD_DIR=%RETVAL%
call :realpath !-I!
set INSTALL_DIR=%RETVAL%

:: Ask for confirmation:
echo VCPKG_ROOT       = %VCPKG_ROOT%
echo Source location  = %SOURCE_DIR%
echo Build location   = %BUILD_DIR%
echo Install location = %INSTALL_DIR%
echo Compiler         = %COMPILER%
echo Architecture     = %ARCHITECTURE%
choice /C:YN /M Continue?
if ERRORLEVEL == 2 goto :usage

set MANIFEST_DIR="%SOURCE_DIR%\vcpkg\ports\simdissdk"

if not exist "%MANIFEST_DIR%\vcpkg.json" (
    set ERROR_MSG=No vcpkg.json manifest found. Run this script from the root folder of the git repository
    goto usage
)

:: Run CMAKE
mkdir %BUILD_DIR%

if exist %BUILD_DIR%\CMakeCache.txt (
    del %BUILD_DIR%\CMakeCache.txt
)

cmake ^
    -S "%SOURCE_DIR%" ^
    -B "%BUILD_DIR%" ^
    -G "%COMPILER%" ^
    -A %ARCHITECTURE% ^
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN_FILE% ^
    -DVCPKG_MANIFEST_DIR="%MANIFEST_DIR%"

goto end

:usage
  if not "%ERROR_MSG%" == "" (
      echo Error: %ERROR_MSG%
  )
  echo Usage: vcpkg-bootstrap.bat -S source_folder -B build_folder -I install_folder -G compiler -A architecture
  echo Example: 
  echo    vcpkg-bootstrap.bat -S . -B ..\build -I ..\install -G "Visual Studio 17 2022" -A x64 

:end
  endlocal
  exit /B


:: Converts a relative path to an absolute path
:realpath
  set RETVAL=%~f1
  exit /B 
