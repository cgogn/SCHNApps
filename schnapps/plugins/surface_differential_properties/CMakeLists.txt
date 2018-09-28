project(plugin_surface_differential_properties
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::surface_differential_properties ALIAS plugin_surface_differential_properties)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/surface_differential_properties.h"
		"${CMAKE_CURRENT_LIST_DIR}/surface_differential_properties.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_normal.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_normal.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_normal.ui"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_curvature.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_curvature.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_compute_curvature.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME surface_differential_properties)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_SDP_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/surface_differential_properties
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
