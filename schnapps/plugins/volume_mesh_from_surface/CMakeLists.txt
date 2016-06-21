add_subdirectory(tetgen)
project(plugin_volume_mesh_from_surface
	LANGUAGES CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(HEADER_FILES
	tetgen_structure_io.h
	volume_mesh_from_surface.h
	volume_mesh_from_surface_dock_tab.h
)

set(SOURCE_FILES
	tetgen_structure_io.cpp
	volume_mesh_from_surface.cpp
	volume_mesh_from_surface_dock_tab.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	tet
	${cgogn_core_LIBRARIES}
	${cgogn_io_LIBRARIES}
)
