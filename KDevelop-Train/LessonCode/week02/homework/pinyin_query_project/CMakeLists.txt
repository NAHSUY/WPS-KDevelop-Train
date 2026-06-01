cmake_minimum_required(VERSION 3.10)
project(pinyin_query_project)

add_executable(build_index build_index.cpp)
add_executable(query query.cpp)

# ---- LevelDB 路径 ----
set(LEVELDB_ROOT "D:/C++_project/thirdparty_install/leveldb" CACHE PATH "Path to leveldb root")
message(STATUS "LEVELDB_ROOT=${LEVELDB_ROOT}")

include_directories(${LEVELDB_ROOT}/include)

find_library(LEVELDB_LIB
  NAMES leveldb libleveldb
  PATHS
    ${LEVELDB_ROOT}
    ${LEVELDB_ROOT}/build
    ${LEVELDB_ROOT}/build/Release
    ${LEVELDB_ROOT}/out/Release
  NO_DEFAULT_PATH
)

message(STATUS "LEVELDB_LIB=${LEVELDB_LIB}")

if(NOT LEVELDB_LIB)
  message(FATAL_ERROR "Cannot find leveldb lib. Build leveldb first and set LEVELDB_ROOT.")
endif()

# ---- 目标 ----
add_executable(leveldb_build leveldb_build.cpp)
add_executable(leveldb_query leveldb_query.cpp)
add_executable(benchmark benchmark.cpp)
add_executable(make_queries make_queries.cpp)
add_executable(memory_query memory_query.cpp)
add_executable(memory_benchmark memory_benchmark.cpp)

target_link_libraries(leveldb_build PRIVATE ${LEVELDB_LIB})
target_link_libraries(leveldb_query PRIVATE ${LEVELDB_LIB})
target_link_libraries(benchmark PRIVATE ${LEVELDB_LIB})

if (WIN32)
  target_link_libraries(query PRIVATE Psapi)
endif()