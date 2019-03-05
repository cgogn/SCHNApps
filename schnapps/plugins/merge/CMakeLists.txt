project(plugin_merge
	LANGUAGES CXX
)

find_package(cgogn_geometry REQUIRED) # for Eigen
find_package(cgogn_io REQUIRED)
find_package(schnapps_core REQUIRED)
find_package(plugin_cmap_provider REQUIRED)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::merge ALIAS plugin_merge)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/merge.h"
		"${CMAKE_CURRENT_LIST_DIR}/merge.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d" EXPORT_NAME merge)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(plugin_merge)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
	cgogn::geometry
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/merge
	COMPONENT schnapps_plugins_headers
	FILES_MATCHING PATTERN "*.h"
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_merge_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/merge
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
