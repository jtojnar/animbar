#
# for cmake and qt see
# 		http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4
# and
#		http://www.bineteri.com/qtwithcmake
#

configure_file (
	"animbar.h.in"
	"${PROJECT_BINARY_DIR}/animbar.h"
)

include_directories("${PROJECT_BINARY_DIR}")

INCLUDE(${QT_USE_FILE})

SET(animbar_SRCS
	main.cpp
	MainWindow.cpp
)

SET(animbar_MOC_HDRS
	MainWindow.h
)

QT4_WRAP_CPP(animbar_MOC_SRCS ${animbar_MOC_HDRS})

ADD_EXECUTABLE(animbar
	${animbar_SRCS} 
	${animbar_MOC_SRCS}
)

TARGET_LINK_LIBRARIES(animbar ${QT_LIBRARIES})
