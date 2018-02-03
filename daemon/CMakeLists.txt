cmake_minimum_required(VERSION 3.1)

project(sp_daemon)
enable_testing()
include_directories(src)
add_subdirectory( src bin )
add_subdirectory( tests tests )
