cmake_minimum_required(VERSION 3.10) # 指定CMake的最低版本要求

# 递归查找week02目录下所有CMakeLists.txt文件
file(GLOB_RECURSE CMAKELISTS ${CMAKE_CURRENT_SOURCE_DIR}/homework/*CMakeLists.txt)

# 遍历所有找到的CMakeLists.txt文件
foreach(CMAKELIST ${CMAKELISTS})
    # 提取CMakeLists.txt文件所在的目录
    get_filename_component(DIRECTORY ${CMAKELIST} DIRECTORY)
    
    # 添加子目录到构建中
    add_subdirectory(${DIRECTORY})
endforeach()