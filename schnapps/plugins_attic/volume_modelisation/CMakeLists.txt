project(plugin_volume_modelisation
	LANGUAGES CXX
)

find_package(cgogn_geometry REQUIRED)
find_package(cgogn_modeling REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::volume_modelisation ALIAS plugin_volume_modelisation)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_modelisation.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_modelisation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/volume_modelisation_docktab.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_modelisation_docktab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/volume_operation.h"
		"${CMAKE_CURRENT_LIST_DIR}/volume_operation.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/volume_modelisation.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME volume_modelisation)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_VOLUME_MODELISATION_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	cgogn_core
	cgogn_geometry
	cgogn_modeling
	Qt5::Core
	Qt5::Widgets
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
