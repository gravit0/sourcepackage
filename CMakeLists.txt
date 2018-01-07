cmake_minimum_required(VERSION 3.1)

project(sourceofpackage)
include_directories(libsp/include)
add_subdirectory( libsp libsp )
add_subdirectory( client client )
add_subdirectory( daemon daemon )
add_subdirectory( sourcepackageinteractive spi )
