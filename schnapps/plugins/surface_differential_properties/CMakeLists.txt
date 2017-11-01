project(plugin_surface_differential_properties
	LANGUAGES CXX
)

#find_package(cgogn_core REQUIRED)
find_package(cgogn_geometry REQUIRED)
#find_package(Qt5 COMPONENTS Widgets REQUIRED)
#find_package(QOGLViewer REQUIRED)

set(HEADER_FILES
	dll.h
	surface_differential_properties.h
	dialog_compute_normal.h
	dialog_compute_curvature.h
)

set(SOURCE_FILES
	surface_differential_properties.cpp
	dialog_compute_normal.cpp
	dialog_compute_curvature.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_SDP_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	${QOGLViewer_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${cgogn_geometry_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QOGLViewer_LIBRARIES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
