#
# for details on commands see for example
#		http://www.cmake.org/cmake/help/cmake-2-8-docs.html#section_Commands
#
# cmake encourages the idea of out-of-source compile, which is not that
# stupid at all. We create a build directory "build", change to and call
# "cmake ../" from there to create the makefiles in the build directory.
# calling make therein gives the executable in the build directory as 
# well. To get rid of all files created by the build process (just as
# make distclean), simply remove the build directory.
#
# Use
#		make VERBOSE = 1
# to get more verbose output or 
#		set(CMAKE_VERBOSE_MAKEFILE ON)
# herein

cmake_minimum_required(VERSION 2.6)

project(animbar) 

set (animbar_VERSION_MAJOR 1)
set (animbar_VERSION_MINOR 0)

FIND_PACKAGE(Qt4 REQUIRED)

add_definitions(-DANIMBAR_DEBUG -Wall)

add_subdirectory(src)

# this will create a target "package_source", hence do 
# "make package_source"
set(CPACK_PACKAGE_VERSION_MAJOR "${animbar_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${animbar_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;ZIP")
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}/*")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${animbar_VERSION_MAJOR}.${animbar_VERSION_MINOR}")
include(CPack)
