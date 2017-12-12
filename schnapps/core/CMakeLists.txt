project(schnapps_core
	LANGUAGES CXX
)

find_package(cgogn_rendering REQUIRED)


set(HEADER_FILES
	dll.h
	schnapps_window.h
	schnapps_window_factory.h
	schnapps.h
	camera.h
	view.h
	view_dialog_list.h
	view_button_area.h
	plugin.h
	plugin_processing.h
	plugin_interaction.h
	map_handler.h
	control_dock_camera_tab.h
	control_dock_plugin_tab.h
	control_dock_map_tab.h
	types.h
	color_combo_box.h
	cells_set.h
	settings.h
	settings_widget.h
	status_bar_output.h
)

set(SOURCE_FILES
	schnapps_window.cpp
	schnapps.cpp
	camera.cpp
	view.cpp
	view_dialog_list.cpp
	view_button_area.cpp
	plugin_interaction.cpp
	plugin.cpp
	plugin_processing.cpp
	map_handler.cpp
	control_dock_camera_tab.cpp
	control_dock_plugin_tab.cpp
	control_dock_map_tab.cpp
	color_combo_box.cpp
	cells_set.cpp
	types.cpp
	settings.cpp
	settings_widget.cpp
	status_bar_output.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt5_add_resources(RCC_FILES
	${SCHNAPPS_PATH}/schnapps/resources/resources.qrc
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${RCC_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE "SCHNAPPS_CORE_DLL_EXPORT")

if (SCHNAPPS_ALIGNED_VEC3)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_USE_ALIGNEDVEC3")
endif()

if (SCHNAPPS_DOUBLE_PRECISION)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_DOUBLE_PRECISION")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC "SCHNAPPS_SINGLE_PRECISION")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")


target_include_directories(${PROJECT_NAME} PUBLIC
	${QOGLViewer_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_BINARY_DIR}
	$<INSTALL_INTERFACE:include/schnapps/core>
)

target_link_libraries(${PROJECT_NAME}
	${cgogn_core_LIBRARIES}
	${cgogn_rendering_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QOGLViewer_LIBRARIES}
)

if(WIN32)
	# copying cgogn's dlls
	foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
		file(GLOB CGOGN_DLLS "${CGOGN_BUILD_OR_INSTALL_PATH}/bin/${CONFIG}/*.dll")
		file(COPY ${CGOGN_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
	endforeach()
	# copying Qt's dlls
	get_target_property(QT5_CORE_RELEASE_DLLS ${Qt5Core_LIBRARIES} IMPORTED_LOCATION_RELEASE)
	get_target_property(QT5_GUI_RELEASE_DLLS ${Qt5Gui_LIBRARIES} IMPORTED_LOCATION_RELEASE)
	get_target_property(QT5_WIDGETS_RELEASE_DLLS ${Qt5Widgets_LIBRARIES} IMPORTED_LOCATION_RELEASE)
	get_target_property(QT5_CORE_DEBUG_DLLS ${Qt5Core_LIBRARIES} IMPORTED_LOCATION_DEBUG)
	get_target_property(QT5_GUI_DEBUG_DLLS ${Qt5Gui_LIBRARIES} IMPORTED_LOCATION_DEBUG)
	get_target_property(QT5_WIDGETS_DEBUG_DLLS ${Qt5Widgets_LIBRARIES} IMPORTED_LOCATION_DEBUG)
	foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
		file(COPY ${QT5_CORE_RELEASE_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		file(COPY ${QT5_GUI_RELEASE_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		file(COPY ${QT5_WIDGETS_RELEASE_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		file(COPY ${QT5_CORE_DEBUG_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		file(COPY ${QT5_GUI_DEBUG_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		file(COPY ${QT5_WIDGETS_DEBUG_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
	endforeach()
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		"settings.json"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(DIRECTORY .
	DESTINATION include/schnapps/core
	FILES_MATCHING PATTERN "*.h"
)

install(TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}Targets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER core)
