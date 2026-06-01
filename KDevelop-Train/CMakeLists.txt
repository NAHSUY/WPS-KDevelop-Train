cmake_minimum_required(VERSION 3.10)  #cmake版本最低3.10

if(WIN32)
    set(CMAKE_SYSTEM_VERSION "10.0.18362.0")
endif()

SET(CMAKE_TOOLCHAIN_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty_install/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(KDevelop-Train)  #添加当前项目名称
set(CMAKE_CXX_STANDARD 17) #设置C++标准
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin") # 设置输出目录为顶层 bin 文件夹

add_subdirectory(Example)
add_subdirectory(LessonCode)
add_subdirectory(ClassExamProject)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtDemo)  #设置启动项

# ======= GTest 单元测试（修复运行时库冲突）=======
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# 关键！强制 GTest 使用 MD 动态运行库（和你的项目保持一致）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# 生成测试程序
add_executable(CoverageTest test.cpp)
target_link_libraries(CoverageTest gtest gtest_main)