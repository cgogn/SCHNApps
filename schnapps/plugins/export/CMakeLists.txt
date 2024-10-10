project(plugin_export
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)
find_package(cgogn_io REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::export ALIAS plugin_export)


target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/export.h"
		"${CMAKE_CURRENT_LIST_DIR}/export.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
			DEBUG_POSTFIX "_d"
			EXPORT_NAME export)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(plugin_export)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	cgogn::io
	schnapps::cmap_provider
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/export
	FILES_MATCHING PATTERN "*.h"
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_export_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/export
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
