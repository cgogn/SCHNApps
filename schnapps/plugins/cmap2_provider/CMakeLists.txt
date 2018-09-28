project(plugin_cmap2_provider
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::cmap2_provider ALIAS plugin_cmap2_provider)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_provider.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_provider.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_handler.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_handler.cpp"
#		"${CMAKE_CURRENT_LIST_DIR}/cmap2_cells_set.h"
#		"${CMAKE_CURRENT_LIST_DIR}/cmap2_cells_set.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_provider_dock_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_provider_dock_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cmap2_provider.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME cmap2_provider)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_CMAP2_PROVIDER_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/cmap2_provider
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
