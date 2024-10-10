project(plugin_extract_surface
	LANGUAGES CXX
)


find_package(schnapps_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(plugin_cmap_provider REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::extract_surface ALIAS plugin_extract_surface)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/extract_surface.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/extract_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/extract_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

generate_export_header(plugin_extract_surface)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	cgogn::io
	schnapps::cmap_provider
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/extract_surface
	COMPONENT schnapps_plugins_headers
	FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_extract_surface_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/extract_surface
	COMPONENT schnapps_plugins_headers
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "plugins")
