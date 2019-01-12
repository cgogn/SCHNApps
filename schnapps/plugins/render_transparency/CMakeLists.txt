if (Qt5Widgets_VERSION VERSION_GREATER 5.5.99)

project(plugin_render_transparency
	LANGUAGES CXX)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::render_transparency ALIAS plugin_render_transparency)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/render_transparency.h"
		"${CMAKE_CURRENT_LIST_DIR}/render_transparency.cpp"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME render_transparency)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_RENDER_TRANSPARENCY_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/render_transparency
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)

endif()
