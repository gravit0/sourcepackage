cmake_minimum_required(VERSION 3.1)

project(libsp)
set( SOURCE_ROOT ./src )
set (CMAKE_CXX_STANDARD 11)
include_directories(include)
aux_source_directory( ${SOURCE_ROOT} SOURCES )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
add_library(libsp SHARED ${SOURCES})
add_library(libsp_static STATIC ${SOURCES})
set_target_properties(libsp_static PROPERTIES OUTPUT_NAME sp)
set_target_properties(libsp PROPERTIES OUTPUT_NAME sp)
