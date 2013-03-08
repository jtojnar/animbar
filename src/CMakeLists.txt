#
# for cmake and qt see
# 		http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4
# and
#		http://www.bineteri.com/qtwithcmake
#

#-----------------------------------------------------------------------
# static runtime linking (is bad, but we don't want to go through the
# pain to distribute the runtime dll).
# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
#-----------------------------------------------------------------------

IF (WIN32)
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
ENDIF (WIN32)

# Furthermore, qt is linked against Multi-threaded dll, so we have to 
# ignore all runtimes except our static Multi-threaded one.
#	http://msdn.microsoft.com/en-us/library/Aa267384 

IF (WIN32)
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libc.lib")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:msvcrt.lib")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcd.lib")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libcmtd.lib")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:msvcrtd.lib")
ENDIF (WIN32)

# if, one day, we want to use the vs2008 sp1 redistributable, see
#	http://forums.winamp.com/showthread.php?threadid=304198

#-----------------------------------------------------------------------
# animbar.h from animbar.h.in
#-----------------------------------------------------------------------

# create animbar.h from animbar.h.in with cmake variable input.
configure_file (
	"animbar.h.in"
	"${PROJECT_BINARY_DIR}/animbar.h"
)

# animbar.h will be in the bin dir
include_directories("${PROJECT_BINARY_DIR}")

#-----------------------------------------------------------------------
# setup sources, mocs etc.
#-----------------------------------------------------------------------

SET(animbar_SRCS
	main.cpp
	MainWindow.cpp
)

IF (WIN32)
	set(animbar_SRCS ${animbar_SRCS} ../animbar.rc)
ENDIF (WIN32)

SET(animbar_MOC_HDRS
	MainWindow.h
)

# moc 'em
QT4_WRAP_CPP(animbar_MOC_SRCS ${animbar_MOC_HDRS})

#-----------------------------------------------------------------------
# What makes up our final target executable. WIN32, MACOSX_BUNDLE that
# some further platform specific things
#-----------------------------------------------------------------------

ADD_EXECUTABLE(animbar
	WIN32 MACOSX_BUNDLE
	${animbar_SRCS} 
	${animbar_MOC_SRCS}
)

TARGET_LINK_LIBRARIES(animbar 
	${QT_LIBRARIES}
)

#-----------------------------------------------------------------------
# Installation setup. This is either for a 
#	make install
# or to generate binary distributions (e.g. nsis)
# most of the code here is from
#	http://www.cmake.org/Wiki/BundleUtilitiesExample
#-----------------------------------------------------------------------

# in the following, we only set up four internal script variables, that
# will be used later on: PLUGIN_DEST_DIR, QTCONF_DEST_DIR, APPS and DIR
SET(PLUGIN_DEST_DIR bin)
SET(QTCONF_DEST_DIR bin)
SET(DIRS ${QT_BINARY_DIR})
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")
IF(APPLE)
    SET(PLUGIN_DEST_DIR QtTest.app/Contents/MacOS)
    SET(QTCONF_DEST_DIR QtTest.app/Contents/Resources)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
ENDIF(APPLE)
IF(WIN32)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe")
ENDIF(WIN32)

#-----------------------------------------------------------------------
# this is the major install command that we need to get binary packaging
# to work, for example
INSTALL(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)

#-----------------------------------------------------------------------
# apart from application plugins, qt provides an interface to have
# plugins to itself, e.g. image format support etc. the following 
# includes all image format plugins. that is what we want.
INSTALL(
	DIRECTORY "${QT_PLUGINS_DIR}/imageformats" 
	DESTINATION ${PLUGIN_DEST_DIR}/plugins 
	COMPONENT Runtime
)

MESSAGE("${QT_PLUGINS_DIR}")

#-----------------------------------------------------------------------
# the following inserts CODE to the INSTALL script to write a qt.conf,
# from what I can tell this just creates and empty qt.conf.
INSTALL(
	CODE "
		file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${QTCONF_DEST_DIR}/qt.conf\" \"\")
    " 
    COMPONENT Runtime
)

#-----------------------------------------------------------------------
# now here happens the real magic. the BundleUtilities copy all Qt
# dependancies.

INSTALL(
	CODE "
		file(GLOB_RECURSE QTPLUGINS
		\"\${CMAKE_INSTALL_PREFIX}/${PLUGIN_DEST_DIR}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
		include(BundleUtilities)
		fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " 
    COMPONENT Runtime
)


