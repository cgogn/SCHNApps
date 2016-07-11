project(schnapps_core
	LANGUAGES CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_rendering REQUIRED)
find_package(Qt5 5.4.0 COMPONENTS Widgets REQUIRED)
find_package(QOGLViewer REQUIRED)

set(HEADER_FILES
	dll.h
	schnapps_window.h
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
)

set(SOURCE_FILES
	schnapps_window.cpp
	schnapps.cpp
	camera.cpp
	view.cpp
	view_dialog_list.cpp
	view_button_area.cpp
	plugin_interaction.cpp
	map_handler.cpp
	control_dock_camera_tab.cpp
	control_dock_plugin_tab.cpp
	control_dock_map_tab.cpp
	color_combo_box.cpp
	cells_set.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt5_add_resources(RCC_FILES
	${SCHNAPPS_PATH}/schnapps/resources/resources.qrc
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${RCC_FILES})

if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++11")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_CORE_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	${QOGLViewer_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
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
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CGOGN_BUILD_PATH}/bin/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()


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
