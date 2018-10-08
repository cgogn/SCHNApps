project(plugin_attribute_editor
	LANGUAGES CXX
)

find_package(schnapps_core REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::attribute_editor ALIAS plugin_attribute_editor)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/attribute_editor.h"
		"${CMAKE_CURRENT_LIST_DIR}/attribute_editor.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/add_attribute_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/add_attribute_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/add_attribute_dialog.ui"
		"${CMAKE_CURRENT_LIST_DIR}/attribute_factory.h"
		"${CMAKE_CURRENT_LIST_DIR}/edit_attribute_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/edit_attribute_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/edit_attribute_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME attribute_editor)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/attribute_editor
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
