cmake_minimum_required(VERSION 3.16)

project(KApple VERSION 0.1 LANGUAGES CXX)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Qt相关设置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找Qt5（必须在add_executable之前）
find_package(Qt5 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Multimedia 
    REQUIRED
)

# 显式列出所有源文件
set(SOURCES
    main.cpp
    widget.cpp
    widget.h
    gamemap.cpp
    gamemap.h
    gamemedia.cpp
    gamemedia.h
    applepool.cpp
    applepool.h
    apple.cpp
    apple.h
    uilayer.cpp
    uilayer.h
    PopupWidget/popwidgetsmanager.cpp
    PopupWidget/popwidgetsmanager.h
    PopupWidget/widgetexit.cpp
    PopupWidget/widgetexit.h
    PopupWidget/widgetfail.cpp
    PopupWidget/widgetfail.h
    PopupWidget/widgetsetup.cpp
    PopupWidget/widgetsetup.h
    PopupWidget/widgetsetupsure.cpp
    PopupWidget/widgetsetupsure.h
    PopupWidget/widgetwin.cpp
    PopupWidget/widgetwin.h
    PopupWidget/myabstractwidget.cpp
    PopupWidget/myabstractwidget.h
    PopupWidget/imagebutton.cpp
    PopupWidget/imagebutton.h
    PopupWidget/public_enum.h
    KAppleSrc.qrc
)

# ★ 关键修改：移除WIN32标志，让Qt处理GUI应用设置
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Multimedia
)

# MSVC编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8 /Zc:__cplusplus)
    # Windows下的subsystem设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# macOS设置
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.KApple"
    )
endif()

# 安装规则
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)