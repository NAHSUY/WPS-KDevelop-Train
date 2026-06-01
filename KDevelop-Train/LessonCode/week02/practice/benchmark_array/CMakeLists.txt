cmake_minimum_required(VERSION 3.10)  
  
project(benchmark_array)  
  
set(CMAKE_CXX_STANDARD 17)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  

set(BENCHMARK_INSTALL_DIR "D:/C++_project/KDevelop-Train/LessonCode/week02/practice/benchmark/build")  
  
find_package(benchmark REQUIRED PATHS ${BENCHMARK_INSTALL_DIR})  

add_executable(benchmark_array cppThread.cpp)  
  
target_link_libraries(benchmark_array benchmark::benchmark)  