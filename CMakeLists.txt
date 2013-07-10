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
#
# on win32, install visual studio express 2008 or 2010 and qt "framework only" 
# for vs2008. Get cmake from www.cmake.org. cmake creates a solution with four
# projects. "animbar" compiles the source and "package" creates the windows
# installer. Only release builds will work on win32 because of dependancy madness.

cmake_minimum_required(VERSION 2.6)

project(animbar) 

#-----------------------------------------------------------------------
# set some variables.
#-----------------------------------------------------------------------

set (animbar_VERSION_MAJOR 1)
set (animbar_VERSION_MINOR 2)

IF(MSVC)
	# on windows, we link agains qmain to have a winmain which let us start
	# without console
	SET(QT_USE_QTMAIN TRUE)
ELSE(MSVC)
	# -Wall with Visual Studio gives us tons of warnings from qt we are
	# not interested in. so we only include if not on win32.
	add_definitions(-Wall)
ENDIF(MSVC)

#-----------------------------------------------------------------------
# setup qt
#-----------------------------------------------------------------------

# On Ubuntu, run cmake with -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake-qt4, the 
# export QT_SELECT mechanism does confuse FindQt4, e.g.
#		cmake ../ -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake-qt4
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtSvg REQUIRED)

INCLUDE(${QT_USE_FILE})

#-----------------------------------------------------------------------
# some all platform options
#-----------------------------------------------------------------------

SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_BUILD_TYPE Release)

#add_definitions(-DANIMBAR_DEBUG)

IF(NOT CMAKE_BUILD_TYPE)
	SET(
		CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE
	)
ENDIF(NOT CMAKE_BUILD_TYPE)


#-----------------------------------------------------------------------
# recurse
#-----------------------------------------------------------------------

add_subdirectory(src)

#-----------------------------------------------------------------------
# icons
#-----------------------------------------------------------------------

SET(animbar_ICONS
	icon/animbar.png
	icon/animbar.svg
	icon/animbar.ico
)

#-----------------------------------------------------------------------
# install the icons

INSTALL(
	FILES ${animbar_ICONS} 
	DESTINATION pixmaps/ 
	COMPONENT Runtime
)

#-----------------------------------------------------------------------
# cpack configuration
#-----------------------------------------------------------------------

# this will create a target "package_source", hence do 
# "make package_source" on linux. for msvc on windows, there will be a
# package build target which creates an installer package with nsis. 
# which is - I have to say - gorgeous.
# see http://batchmake.org/Wiki/CMake:CPackConfiguration for a list of
# variables.
set(CPACK_PACKAGE_VERSION_MAJOR "${animbar_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${animbar_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "")
IF(NOT WIN32)
	set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;ZIP")
	set(CPACK_BINARY_GENERATOR "")
ELSE(NOT WIN32)
	set(CPACK_SOURCE_GENERATOR "")
	set(CPACK_BINARY_GENERATOR "NSIS")
ENDIF(NOT WIN32)
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}/*" "CMakeLists.txt..*" "svn")
set(CPACK_PACKAGE_VERSION "${animbar_VERSION_MAJOR}.${animbar_VERSION_MINOR}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_NSIS_URL_INFO_ABOUT "http:////animbar.mnim.org")
# This makes NSIS asks if we want to add the animbar install dir to the
# system path AND if a desktop icon shall be created  
set(CPACK_NSIS_MODIFY_PATH "ON")
# setting the following gives us a start menu entry on windows.
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "${PROJECT_NAME}")

include(CPack)

