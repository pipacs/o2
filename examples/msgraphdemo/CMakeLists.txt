cmake_minimum_required(VERSION 2.8.11)

project( msgraphexample )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(o2_WITH_QT5)
    # Qt5 packages find their own dependencies.
    find_package(Qt5Core    REQUIRED)
    find_package(Qt5Gui     REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(Qt5Widgets REQUIRED)
else(o2_WITH_QT5)
    set(QT_USE_QTNETWORK true)
    set(QT_USE_QTSCRIPT true)
    find_package(Qt4 REQUIRED)
endif(o2_WITH_QT5)

if (NOT o2_WITH_QT5)
    include( ${QT_USE_FILE} )
endif(NOT o2_WITH_QT5)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} "../../src" )

set(msgraph_SRCS
    main.cpp
    msgraphdemo.cpp
    msgraphdemo.h
)

if(NOT o2_WITH_QT5)
    add_definitions(${QT4_DEFINITIONS})
endif(NOT o2_WITH_QT5)

add_executable( msgraphexample ${msgraph_SRCS} )

if(o2_WITH_QT5)
    qt5_use_modules( msgraphexample Core Gui Network Widgets )
    target_link_libraries( msgraphexample o2 )
else(o2_WITH_QT5)
    target_link_libraries( msgraphexample ${QT_LIBRARIES} o2 )
endif(o2_WITH_QT5)
