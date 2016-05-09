cmake_minimum_required(VERSION 2.8.11)

project(o2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(o2_WITH_QT5)
    find_package(Qt5 COMPONENTS Core Network REQUIRED)
else(o2_WITH_QT5)
    set(QT_USE_QTNETWORK true)
    set(QT_USE_QTSCRIPT true)
    find_package(Qt4 REQUIRED)
endif(o2_WITH_QT5)
#find_package(QJson REQUIRED)

if (NOT o2_WITH_QT5)
    include( ${QT_USE_FILE} )
endif(NOT o2_WITH_QT5)

set( o2_SRCS
    o2.cpp
    o2reply.cpp
    o2replyserver.cpp
    o2requestor.cpp
    o2simplecrypt.cpp
    o0settingsstore.cpp
    o0baseauth.cpp
    o0abstractstore.h
    o0globals.h
)

if(o2_WITH_OAUTH1)
    set( o2_SRCS
        ${o2_SRCS}
        o1.cpp
        o1requestor.cpp
        o1timedreply.cpp
    )
endif(o2_WITH_OAUTH1)

if(o2_WITH_TWITTER)
    set( o2_SRCS
        ${o2_SRCS}
        o1twitter.h
        oxtwitter.cpp
    )
endif(o2_WITH_TWITTER)

if(o2_WITH_DROPBOX)
    set( o2_SRCS
        ${o2_SRCS}
        o1dropbox.h
    )
endif(o2_WITH_DROPBOX)

if(o2_WITH_GOOGLE)
    set( o2_SRCS
        ${o2_SRCS}
        o2gft.cpp
    )
endif(o2_WITH_GOOGLE)

if(o2_WITH_FACEBOOK)
    set( o2_SRCS
        ${o2_SRCS}
        o2facebook.cpp
    )
endif(o2_WITH_FACEBOOK)

if(o2_WITH_SKYDRIVE)
    set( o2_SRCS
        ${o2_SRCS}
        o2skydrive.cpp
    )
endif(o2_WITH_SKYDRIVE)

if(o2_WITH_FLICKR)
    set( o2_SRCS
        ${o2_SRCS}
        o1flickr.h
    )
endif(o2_WITH_FLICKR)

if(o2_WITH_HUBIC)
    set( o2_SRCS
        ${o2_SRCS}
        o2hubic.cpp
    )
endif(o2_WITH_HUBIC)

if(NOT o2_WITH_QT5)
    add_definitions(${QT4_DEFINITIONS})
endif(NOT o2_WITH_QT5)

add_library( o2 ${o2_SRCS} )

if(o2_WITH_QT5)
    target_link_libraries( o2 Qt5::Core Qt5::Network)
else(o2_WITH_QT5)
    target_link_libraries( o2 ${QT_LIBRARIES} )
endif(o2_WITH_QT5)
