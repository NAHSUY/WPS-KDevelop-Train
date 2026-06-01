cmake_minimum_required(VERSION 3.16)

project(KSPACE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 使用 Qt5
find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia Core)

set(PROJECT_SOURCES
    KSpaceSrc.qrc
    main.cpp
    gamemedia.cpp gamemedia.h
    widget.cpp widget.h

    PopupWidget/imagebutton.cpp PopupWidget/imagebutton.h
    PopupWidget/mapview.cpp PopupWidget/mapview.h
    PopupWidget/widgetranklist.cpp PopupWidget/widgetranklist.h
    PopupWidget/myabstractwidget.h
    PopupWidget/popwidgetsmanager.cpp PopupWidget/popwidgetsmanager.h
    PopupWidget/public_enum.h
    PopupWidget/statusbar.cpp PopupWidget/statusbar.h
    PopupWidget/widgetexit.cpp PopupWidget/widgetexit.h
    PopupWidget/widgetsetup.cpp PopupWidget/widgetsetup.h
    PopupWidget/widgetsetupsure.cpp PopupWidget/widgetsetupsure.h
    PopupWidget/enemy.h PopupWidget/enemy.cpp
    PopupWidget/bullet.h PopupWidget/bullet.cpp
    PopupWidget/player.h PopupWidget/player.cpp
)

if(ANDROID)
    add_library(KSPACE SHARED
        ${PROJECT_SOURCES}
    )
else()
    add_executable(KSPACE
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(KSPACE PRIVATE
    Qt5::Widgets
    Qt5::Multimedia
    Qt5::Core
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(QT_VERSION VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.KSPACE)
endif()

set_target_properties(KSPACE PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS KSPACE
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)