project(plugin_cmap0_provider
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::cmap0_provider ALIAS plugin_cmap0_provider)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_provider.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_provider.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_handler.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_handler.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_cells_set.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_cells_set.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_provider_dock_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_provider_dock_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap0_provider.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME cmap0_provider)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_CMAP0_PROVIDER_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/cmap0_provider
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
