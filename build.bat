@echo off
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Visual Studio Installer vswhere.exe was not found.
    exit /b 1
)

for /f "delims=" %%I in ('call "%VSWHERE%" -latest -version "[18.0,19.0)" -products * -property installationPath') do set "VS_INSTALL=%%I"
if not defined VS_INSTALL (
    echo Visual Studio 2026 was not found.
    exit /b 1
)

set "CMAKE_EXE=%VS_INSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist "%CMAKE_EXE%" (
    echo The CMake component is missing from Visual Studio 2026.
    exit /b 1
)

pushd "%~dp0"
"%CMAKE_EXE%" --fresh -S . -B build -G "Visual Studio 18 2026" -A x64
set "RESULT=%ERRORLEVEL%"
if "%RESULT%"=="0" if exist "build\FortuneRenderer.sln" del /q "build\FortuneRenderer.sln"
popd
exit /b %RESULT%

