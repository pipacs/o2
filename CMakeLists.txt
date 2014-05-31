cmake_minimum_required(VERSION 2.8.11)

project(o2)

add_subdirectory(src)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(BUILD_EXAMPLES)
