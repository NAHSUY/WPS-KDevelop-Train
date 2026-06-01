cmake_minimum_required(VERSION 3.10)

project(Drawing)

# 👇 关键：添加 WIN32 表示这是 GUI 程序（使用 WinMain）
add_executable(${PROJECT_NAME} WIN32 Drawing.cpp)

# 链接 Windows 图形库
target_link_libraries(${PROJECT_NAME}
    user32
    gdi32
)