cmake_minimum_required(VERSION 2.8.11)

project(o2)

option(o2_WITH_QT5 "Use Qt5" ON)

option(o2_SHOW_TRACE "Show debugging messages" OFF)
if(NOT o2_SHOW_TRACE)
  add_definitions(-DQT_NO_DEBUG_OUTPUT=1)
endif()

option(o2_WITH_TWITTER "Authenticate with Twitter" OFF)
option(o2_WITH_DROPBOX "Authenticate with Dropbox" OFF)
option(o2_WITH_GOOGLE "Authenticate with Google" OFF)
option(o2_WITH_FACEBOOK "Authenticate with Facebook" OFF)
option(o2_WITH_SKYDRIVE "Authenticate with SkyDrive" OFF)
option(o2_WITH_FLICKR "Authenticate with Flickr" OFF)
option(o2_WITH_HUBIC "Authenticate with Hubic" OFF)

option(o2_WITH_OAUTH1 "Include OAuth1 authentication" OFF)
if(o2_WITH_TWITTER OR o2_WITH_DROPBOX OR o2_WITH_FLICKR)
  set(o2_WITH_OAUTH1 ON)
endif()

option(o2_BUILD_EXAMPLES "Build examples" OFF)

add_subdirectory(src)

if(o2_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(o2_BUILD_EXAMPLES)
