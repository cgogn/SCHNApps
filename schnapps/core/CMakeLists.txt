project(schnapps_core
	LANGUAGES CXX
)

set(HEADER_FILES
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
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

qt5_add_resources(RCC_FILES
	${SCHNAPPS_PATH}/schnapps/resources/resources.qrc
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${RCC_FILES})

# use of target_compile_options to have a transitive c++11 flag
if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++11")
endif()
if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-D_USE_MATH_DEFINES")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_THIRDPARTY_QOGLVIEWER_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_EIGEN3_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	${CMAKE_CURRENT_BINARY_DIR}
	$<INSTALL_INTERFACE:include/schnapps/core>
)

target_link_libraries(${PROJECT_NAME}
	${CGOGN_PATH}/../CGoGN_2-build/lib/libcgogn_core.so
	${CGOGN_PATH}/../CGoGN_2-build/lib/libcgogn_rendering.so
	QOGLViewer
	Qt5::Widgets
)

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
