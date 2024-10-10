project(plugin_surface_modelisation
	LANGUAGES CXX
)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

find_package(schnapps_core REQUIRED)
find_package(cgogn_modeling REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::surface_modelisation ALIAS plugin_surface_modelisation)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/surface_modelisation.h"
		"${CMAKE_CURRENT_LIST_DIR}/surface_modelisation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.ui"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.ui"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_remeshing.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_remeshing.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_remeshing.ui"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_filtering.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_filtering.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_filtering.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME surface_modelisation)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(plugin_surface_modelisation)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
	cgogn::modeling
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/surface_modelisation
	COMPONENT schnapps_plugins_headers
	FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_surface_modelisation_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/surface_modelisation
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
