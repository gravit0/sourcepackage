cmake_minimum_required(VERSION 2.8)

project(sp_daemon)
#add_executable(${PROJECT_NAME} "src/main.cpp" "src/boost-server.cpp" "src/utils.cpp" "src/basecmds.cpp" "src/testcmds.cpp" "src/basefuncions.cpp" "src/service.cpp" "src/tcpclient.cpp" "src/memcached.cpp")
set( SOURCE_ROOT ./src )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
add_executable(${PROJECT_NAME} ${SOURCES})
#include_directories(SOURCE_ROOT ./src)
#target_link_libraries(${PROJECT_NAME})
