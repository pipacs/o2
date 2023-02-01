cmake_minimum_required(VERSION 3.16)

project(facebookexampleexternalinterceptorexample)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets WebEngineCore WebEngineWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network WebEngineCore WebEngineWidgets)

add_executable(${PROJECT_NAME} WIN32
    main.cpp
    fbdemo.cpp
    fbdemo.h
    webenginepage.cpp
    webenginepage.h
    webwindow.cpp
    webwindow.h
    webwindow.ui
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    o2

    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::WebEngineCore
    Qt${QT_MAJOR_VERSION}::WebEngineWidgets
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
endif()
