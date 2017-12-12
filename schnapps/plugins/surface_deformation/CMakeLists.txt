project(plugin_surface_deformation
	LANGUAGES CXX
)

find_package(cgogn_core REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

find_package(OpenNL REQUIRED)

set(HEADER_FILES
	dll.h
	map_parameters.h
	surface_deformation.h
	surface_deformation_dock_tab.h
)

set(SOURCE_FILES
	surface_deformation.cpp
	surface_deformation_dock_tab.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_SURFACE_DEFORMATION_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_OPEN_NL_INCLUDE_DIR}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${Qt5Core_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${OpenNL_LIBRARIES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
