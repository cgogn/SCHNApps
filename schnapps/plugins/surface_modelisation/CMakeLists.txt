project(plugin_surface_modelisation
	LANGUAGES CXX
)

find_package(cgogn_modeling REQUIRED)

set(HEADER_FILES
	dll.h
	surface_modelisation.h
	dialog_decimation.h
	dialog_subdivision.h
	dialog_remeshing.h
	dialog_filtering.h
)

set(SOURCE_FILES
	surface_modelisation.cpp
	dialog_decimation.cpp
	dialog_subdivision.cpp
	dialog_remeshing.cpp
	dialog_filtering.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_SURFACE_MODELISATION_DLL_EXPORT")


target_include_directories(${PROJECT_NAME} PUBLIC
	${QOGLViewer_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${cgogn_modeling_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QOGLViewer_LIBRARIES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
