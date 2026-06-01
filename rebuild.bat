@echo off
cd /d "%~dp0"

if exist build (
    echo Deleting old build...
    rd /s /q build
)
echo Creating new build directory...
mkdir build
cd build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cmake -G "Visual Studio 17 2022" -A x64 ../KDevelop-Train

if errorlevel 1 (
    echo CMake failed!
    pause
    exit /b 1
)

msbuild KDevelop-Train.sln /p:Configuration=Debug /p:Platform=x64

pause