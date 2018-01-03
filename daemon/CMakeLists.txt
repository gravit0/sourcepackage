cmake_minimum_required(VERSION 3.1)

project(sp_daemon)
enable_testing()
include_directories(src)
add_subdirectory( src build/src )
add_subdirectory( tests build/tests )
