project(schnapps_core
	LANGUAGES CXX
)

find_package(cgogn_rendering REQUIRED)
find_package(QOGLViewer REQUIRED)

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::core ALIAS schnapps_core)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/schnapps_window.h"
		"${CMAKE_CURRENT_LIST_DIR}/schnapps_window.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/schnapps_window_factory.h"
		"${CMAKE_CURRENT_LIST_DIR}/schnapps.h"
		"${CMAKE_CURRENT_LIST_DIR}/schnapps.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/camera.h"
		"${CMAKE_CURRENT_LIST_DIR}/camera.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/view.h"
		"${CMAKE_CURRENT_LIST_DIR}/view.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/view_dialog_list.h"
		"${CMAKE_CURRENT_LIST_DIR}/view_dialog_list.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/view_button_area.h"
		"${CMAKE_CURRENT_LIST_DIR}/view_button_area.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/plugin.h"
		"${CMAKE_CURRENT_LIST_DIR}/plugin.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/plugin_processing.h"
		"${CMAKE_CURRENT_LIST_DIR}/plugin_processing.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/plugin_interaction.h"
		"${CMAKE_CURRENT_LIST_DIR}/plugin_interaction.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/map_handler.h"
		"${CMAKE_CURRENT_LIST_DIR}/map_handler.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_camera_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_camera_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_plugin_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_plugin_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_map_tab.h"
		"${CMAKE_CURRENT_LIST_DIR}/control_dock_map_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types.h"
		"${CMAKE_CURRENT_LIST_DIR}/types.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/color_combo_box.h"
		"${CMAKE_CURRENT_LIST_DIR}/color_combo_box.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/cells_set.h"
		"${CMAKE_CURRENT_LIST_DIR}/cells_set.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/settings.h"
		"${CMAKE_CURRENT_LIST_DIR}/settings.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/settings_widget.h"
		"${CMAKE_CURRENT_LIST_DIR}/settings_widget.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/status_bar_output.h"
		"${CMAKE_CURRENT_LIST_DIR}/status_bar_output.cpp"
		"${SCHNAPPS_SOURCE_DIR}/schnapps/resources/resources.qrc"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE "SCHNAPPS_CORE_DLL_EXPORT")

if (SCHNAPPS_ALIGNED_VEC3)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_USE_ALIGNEDVEC3")
endif()

if (SCHNAPPS_DOUBLE_PRECISION)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_DOUBLE_PRECISION")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_SINGLE_PRECISION")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										BUNDLE TRUE
										EXPORT_NAME core)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${PROJECT_NAME}
	cgogn::core
	cgogn::rendering
	Qt5::Widgets
	cgogn::QOGLViewer
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		"settings.json"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# the trailing slash is IMPORTANT, see https://cmake.org/pipermail/cmake/2009-December/033850.html
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/core
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER core)
