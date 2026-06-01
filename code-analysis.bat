@echo off
echo ==============================================
echo  开始运行全套代码分析工具
echo ==============================================

D:
cd D:\C++_project

echo.
echo [1/4] 编译项目 (CMake+Ninja)
cd build_ninja
ninja
cd ..

echo.
echo [2/4] 运行 clang-tidy 代码质量检查
python "D:\Tool\LLVM\bin\run-clang-tidy" -clang-tidy-binary "D:\Tool\LLVM\bin\clang-tidy.exe" -checks="-*,bugprone-*,modernize-*,performance-*,readability-*" -p build_ninja > clang-tidy-report.txt 2>&1

echo.
echo [3/4] 运行 CppCheck 安全检查
cppcheck KDevelop-Train --enable=all --inconclusive --output-file=cppcheck-report.txt

echo.
echo [4/4] 运行代码覆盖率分析
OpenCppCoverage --sources D:\C++_project\KDevelop-Train --modules D:\C++_project\build_ninja\bin --export_type html:D:\C++_project\coverage_report -- D:\C++_project\build_ninja\bin\KApple.exe

echo.
echo ==============================================
echo  全部分析完成！
echo  报告在 D:\C++_project
echo ==============================================

pause