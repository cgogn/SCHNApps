project(plugin_attribute_editor
	LANGUAGES CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_geometry REQUIRED) #for Eigen
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(HEADER_FILES
	dll.h
	attribute_editor.h
	add_attribute_dialog.h
	attribute_factory.h
	edit_attribute_dialog.h
)

set(SOURCE_FILES
	add_attribute_dialog.cpp
	attribute_editor.cpp
	edit_attribute_dialog.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${cgogn_geometry_LIBRARIES}
	${Qt5Core_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
)
