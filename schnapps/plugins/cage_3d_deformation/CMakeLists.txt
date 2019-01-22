project(plugin_cage_3d_deformation
	LANGUAGES CXX
)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::cage_3d_deformation ALIAS plugin_cage_3d_deformation)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/map_parameters.h"
		"${CMAKE_CURRENT_LIST_DIR}/cage_3d_deformation.h"
		"${CMAKE_CURRENT_LIST_DIR}/cage_3d_deformation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_cage_3d_deformation.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_cage_3d_deformation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_cage_3d_deformation.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
									   DEBUG_POSTFIX "_d"
									   EXPORT_NAME cage_3d_deformation)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(plugin_cage_3d_deformation)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/cage_3d_deformation
	FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin_cage_3d_deformation_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/cage_3d_deformation
	COMPONENT schnapps_plugins_headers
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
