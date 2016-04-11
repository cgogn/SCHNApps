set(SCHNAPPS_THIRDPARTY_QOGLVIEWER_INCLUDE_DIR
	"${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "QOGLViewer directory"
)

project(QOGLViewer)
find_package(OpenGL REQUIRED)
find_package(Qt5Widgets 5.4.0 REQUIRED)

set(CMAKE_AUTOMOC ON)

add_subdirectory(QOGLViewer)
