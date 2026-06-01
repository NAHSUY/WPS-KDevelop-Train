cmake_minimum_required(VERSION 3.16)

project(GameSelector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    GameSelector.qrc
    main.cpp
    gameselector.cpp
    gameselector.h
)

add_executable(GameSelector
    ${PROJECT_SOURCES}
)

target_link_libraries(GameSelector PRIVATE
    Qt5::Widgets
)

set_target_properties(GameSelector PROPERTIES
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS GameSelector
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
