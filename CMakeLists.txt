cmake_minimum_required(VERSION 3.1)

project(sp_daemon)
set( SOURCE_ROOT ./src )
set (CMAKE_CXX_STANDARD 11)
aux_source_directory( ${SOURCE_ROOT} SOURCES )
add_executable(${PROJECT_NAME} ${SOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Wall -O2")
target_link_libraries(${PROJECT_NAME} -pthread)
