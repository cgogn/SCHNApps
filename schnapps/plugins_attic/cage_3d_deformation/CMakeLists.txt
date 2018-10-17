project(plugin_cage_3d_deformation
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::cage_3d_deformation ALIAS plugin_cage_3d_deformation)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
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
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/cage_3d_deformation
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
