project(plugin_volume_render
	LANGUAGES CXX
)

find_package(cgogn_rendering REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::volume_render ALIAS plugin_volume_render)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/map_parameters.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render_dock_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render_dock_tab.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME volume_render)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_VOLUME_RENDER_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	${QOGLViewer_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)


target_link_libraries(${PROJECT_NAME}
	schnapps_core
	cgogn_core
	cgogn_rendering
	Qt5::Widgets
	QOGLViewer
)


set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
