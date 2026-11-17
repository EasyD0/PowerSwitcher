@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "BUILD_OUT="

where cmake >nul 2>nul
if errorlevel 1 goto no_cmake

where cl >nul 2>nul
if not errorlevel 1 goto build_msvc

call :find_vs
if not defined VSPATH goto try_mingw

call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
rem vcvars64 can return a non-zero errorlevel because vswhere is missing from
rem its PATH, so check for the compiler itself instead.
where cl >nul 2>nul
if not errorlevel 1 goto build_msvc

:try_mingw
call :find_mingw
if defined MINGW_BIN goto build_mingw

goto no_compiler

:build_msvc
echo [*] Toolchain: MSVC
rem CMake 4.3 + MSVC 14.50 emits a broken RC rule for Ninja ("RC" as the
rem command), so pick the newest Visual Studio generator explicitly instead
rem of relying on whatever CMake considers its default.
call :find_vs_generator
if not defined VS_GEN (
    echo [!] No Visual Studio generator found in this CMake installation.
    goto failed
)
echo [*] Generator: %VS_GEN%
cmake -S "%~dp0." -B "%~dp0build" -G "%VS_GEN%" -A x64
if errorlevel 1 goto failed
cmake --build "%~dp0build" --config Release
if errorlevel 1 goto failed
set "BUILD_OUT=%~dp0build\bin\PowerSwitcher.exe"
goto done

:build_mingw
echo [*] Toolchain: MinGW (%MINGW_BIN%)
set "PATH=%MINGW_BIN%;%PATH%"
cmake -S "%~dp0." -B "%~dp0build-mingw" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto failed
cmake --build "%~dp0build-mingw"
if errorlevel 1 goto failed
set "BUILD_OUT=%~dp0build-mingw\bin\PowerSwitcher.exe"
goto done

:done
echo.
echo [+] Built: %BUILD_OUT%
exit /b 0

:failed
echo.
echo [!] Build failed. See the output above.
exit /b 1

:find_vs_generator
set "VS_GEN="
for /f "tokens=1,2,3,4" %%a in ('cmake --help ^| findstr /r /c:"Visual Studio [0-9][0-9]* [0-9][0-9][0-9][0-9]"') do (
    if not defined VS_GEN set "VS_GEN=%%a %%b %%c %%d"
)
exit /b 0

:find_vs
set "VSPATH="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" exit /b 0
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
exit /b 0

:find_mingw
set "MINGW_BIN="
if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "MINGW_BIN=C:\msys64\ucrt64\bin"
    exit /b 0
)
if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "MINGW_BIN=C:\msys64\mingw64\bin"
    exit /b 0
)
if exist "C:\msys64\clang64\bin\clang++.exe" (
    set "MINGW_BIN=C:\msys64\clang64\bin"
    exit /b 0
)
for /f "delims=" %%i in ('where g++ 2^>nul') do (
    for %%j in ("%%i") do set "MINGW_BIN=%%~dpj"
    exit /b 0
)
exit /b 0

:no_cmake
echo [!] CMake was not found on PATH.
echo     Install it from https://cmake.org/download/ or run the "C++ CMake tools"
echo     component of Visual Studio Build Tools.
exit /b 1

:no_compiler
echo [!] No C++ compiler found.
echo     Install "Desktop development with C++" in Visual Studio Build Tools,
echo     or install MSYS2 with the mingw-w64-ucrt-x86_64-gcc package.
exit /b 1
