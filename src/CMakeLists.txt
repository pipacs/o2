cmake_minimum_required(VERSION 2.8.11)

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
)

set( o2_HDRS
    o2.h
    o2reply.h
    o2replyserver.h
    o2requestor.h
    o0abstractstore.h
    o0baseauth.h
    o0globals.h
    o0requestparameter.h
    o0settingsstore.h
    o0simplecrypt.h
)

if(o2_WITH_OAUTH1)
    set( o2_SRCS
        ${o2_SRCS}
        o1.cpp
        o1requestor.cpp
        o1timedreply.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o1.h
        o1requestor.h
        o1timedreply.h
        o1freshbooks.h
    )
endif(o2_WITH_OAUTH1)

if(o2_WITH_TWITTER)
    set( o2_SRCS
        ${o2_SRCS}
        oxtwitter.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o1twitter.h
        oxtwitter.h
    )
endif(o2_WITH_TWITTER)

if(o2_WITH_DROPBOX)
    set( o2_HDRS
        ${o2_HDRS}
        o1dropbox.h
    )
endif(o2_WITH_DROPBOX)

if(o2_WITH_GOOGLE)
    set( o2_SRCS
        ${o2_SRCS}
        o2gft.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2gft.h
    )
endif(o2_WITH_GOOGLE)

if(o2_WITH_FACEBOOK)
    set( o2_SRCS
        ${o2_SRCS}
        o2facebook.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2facebook.h
    )
endif(o2_WITH_FACEBOOK)

if(o2_WITH_SKYDRIVE)
    set( o2_SRCS
        ${o2_SRCS}
        o2skydrive.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2skydrive.h
    )
endif(o2_WITH_SKYDRIVE)

if(o2_WITH_FLICKR)
    set( o2_HDRS
        ${o2_HDRS}
        o1flickr.h
    )
endif(o2_WITH_FLICKR)

if(o2_WITH_HUBIC)
    set( o2_SRCS
        ${o2_SRCS}
        o2hubic.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2hubic.h
    )
endif(o2_WITH_HUBIC)

if(o2_WITH_SPOTIFY)
    set( o2_SRCS
        ${o2_SRCS}
        o2spotify.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2spotify.h
    )
endif(o2_WITH_SPOTIFY)

if(o2_WITH_SURVEYMONKEY)
    set( o2_SRCS
        ${o2_SRCS}
        o2surveymonkey.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2surveymonkey.h
    )
endif(o2_WITH_SURVEYMONKEY)

if(NOT o2_WITH_QT5)
    add_definitions(${QT4_DEFINITIONS})
endif(NOT o2_WITH_QT5)

if(BUILD_SHARED_LIBS AND APPLE AND POLICY CMP0042) # in CMake >= 2.8.12
    cmake_policy(SET CMP0042 OLD)
    set(CMAKE_MACOSX_RPATH OFF) # don't embed @rpath in install name
endif(BUILD_SHARED_LIBS AND APPLE AND POLICY CMP0042)

add_library( o2 ${o2_SRCS} ${o2_HDRS} )

if(o2_WITH_QT5)
    target_link_libraries( o2 Qt5::Core Qt5::Network)
else(o2_WITH_QT5)
    target_link_libraries( o2 ${QT_LIBRARIES} )
endif(o2_WITH_QT5)

if(BUILD_SHARED_LIBS)
    if(APPLE)
      set_target_properties(o2
          PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib
      )
    endif(APPLE)
    set_target_properties(o2
        PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${API_VERSION}
    )
else(BUILD_SHARED_LIBS)
    # needed for statically linked o2 in shared libs on x86_64
    set_target_properties(o2
        PROPERTIES POSITION_INDEPENDENT_CODE TRUE
    )
endif(BUILD_SHARED_LIBS)

install(TARGETS o2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${o2_LIB_SUFFIX}
    ARCHIVE DESTINATION lib${o2_LIB_SUFFIX}
)

install(FILES ${o2_HDRS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/o2
)
