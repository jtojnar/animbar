#
# for cmake and qt see
# 		http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4
# and
#		http://www.bineteri.com/qtwithcmake
#

# create animbar.h from animbar.h.in with cmake variable input.
configure_file (
	"animbar.h.in"
	"${PROJECT_BINARY_DIR}/animbar.h"
)

# animbar.h will be in the bin dir
include_directories("${PROJECT_BINARY_DIR}")

INCLUDE(${QT_USE_FILE})

SET(animbar_SRCS
	main.cpp
	MainWindow.cpp
)

SET(animbar_MOC_HDRS
	MainWindow.h
)

# moc 'em
QT4_WRAP_CPP(animbar_MOC_SRCS ${animbar_MOC_HDRS})

# we have to set WIN32 to ADD_EXECUTABLE to start without console
IF(WIN32)
	SET(ADD_EXECUTABLE_OS_SPECIFIC WIN32)
ENDIF(WIN32)

IF (APPLE)
	SET(ADD_EXECUTABLE_OS_SPECIFIC MACOSX_BUNDLE)
ENDIF (APPLE)

ADD_EXECUTABLE(animbar
	${ADD_EXECUTABLE_OS_SPECIFIC}
	${animbar_SRCS} 
	${animbar_MOC_SRCS}
)

TARGET_LINK_LIBRARIES(animbar ${QT_LIBRARIES})
