project(plugin_export
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)
find_package(cgogn_io REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::export ALIAS plugin_export)


target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/export.h"
		"${CMAKE_CURRENT_LIST_DIR}/export.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/export_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME export)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_EXPORT_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	cgogn::io
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/export
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
