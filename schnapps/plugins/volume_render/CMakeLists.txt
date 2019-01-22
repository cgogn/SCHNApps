project(plugin_volume_render
	LANGUAGES CXX
)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

find_package(schnapps_core REQUIRED)
find_package(plugin_cmap_provider REQUIRED)

define_property(TARGET PROPERTY USE_TRANSPARENCY BRIEF_DOCS "use transparency or not" FULL_DOCS "use transparency or not")

find_package(cgogn_rendering REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::volume_render ALIAS plugin_volume_render)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/map_parameters.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render.ui"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render_dock_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_render_dock_tab.cpp"
)

if (USE_TRANSPARENCY)
	set_target_properties(${PROJECT_NAME} PROPERTIES USE_TRANSPARENCY "ON")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES USE_TRANSPARENCY "OFF")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME volume_render)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_VOLUME_RENDER_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
	$<$<BOOL:$<TARGET_PROPERTY:USE_TRANSPARENCY>>:plugin_render_transparency>
)

generate_export_header(plugin_volume_render)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/volume_render
	FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_volume_render_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/volume_render
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
