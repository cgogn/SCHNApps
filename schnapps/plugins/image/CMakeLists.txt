project(plugin_image
	LANGUAGES CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(QOGLViewer REQUIRED)

find_package(CGAL COMPONENTS Core ImageIO)
if(CGAL_FOUND)
	include(${CGAL_USE_FILE})
	find_package(MPFR REQUIRED)
	find_package(GMP REQUIRED)
	find_package(Boost REQUIRED)
endif(CGAL_FOUND)

set(HEADER_FILES
	dll.h
	image.h
	image_dock_tab.h

)

set(SOURCE_FILES
	image.cpp
	image_dock_tab.cpp
)

if(CGAL_FOUND)
	set(HEADER_FILES
		${HEADER_FILES}
		cgal/cgal_image.h
		)
	set(SOURCE_FILES
		${SOURCE_FILES}
		cgal/cgal_image.cpp
		)
endif(CGAL_FOUND)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_IMAGE_DLL_EXPORT")
if(CGAL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DPLUGIN_IMAGE_WITH_CGAL")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGAL_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

#target_compile_definitions(${PROJECT_NAME} PUBLIC "-Dcimg_display=0")

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${cgogn_io_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QOGLViewer_LIBRARIES}
	${CGAL_ImageIO_LIBRARY}
)
