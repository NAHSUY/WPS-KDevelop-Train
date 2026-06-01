mkdir build
cd build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake  -G "Visual Studio 17 2022"  -A x64 -DCMAKE_BUILD_TYPE=Release  ../KDevelop-Train
for %%i in (*.sln) do msbuild /m "%%i" /p:Platform=x64 /p:Configuration=Release
cd ../
pause