cmake_minimum_required(VERSION 3.10)

project(GMockExample)

add_executable(${PROJECT_NAME})

find_package(GTest CONFIG REQUIRED)

#向工程添加源码
target_sources(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/foo.h
)

# 头文件搜索路径（当前目录已默认；若需 person 子目录则取消下行注释并确保该目录存在）
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/person)

# 链接 Google Test 到测试目标
target_link_libraries(${PROJECT_NAME} PUBLIC
    GTest::gmock 
	GTest::gtest 
	GTest::gmock_main 
	GTest::gtest_main
  )

