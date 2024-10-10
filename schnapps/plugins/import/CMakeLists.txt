project(plugin_import
	LANGUAGES CXX
)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

find_package(schnapps_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(plugin_cmap_provider REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::import ALIAS plugin_import)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/import.h"
		"${CMAKE_CURRENT_LIST_DIR}/import.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d" EXPORT_NAME import)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(plugin_import)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
	cgogn::io
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/import
	COMPONENT schnapps_plugins_headers
	FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_import_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/import
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
