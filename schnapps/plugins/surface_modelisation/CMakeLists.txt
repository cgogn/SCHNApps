project(plugin_surface_modelisation
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)
find_package(cgogn_modeling REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::attribute_editor ALIAS plugin_attribute_editor)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/surface_modelisation.h"
		"${CMAKE_CURRENT_LIST_DIR}/surface_modelisation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_decimation.ui"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.h"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/dialog_subdivision.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME attribute_editor)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_SURFACE_MODELISATION_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	cgogn::modeling
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/surface_modelisation
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
