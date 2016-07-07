add_subdirectory(tetgen)

project(plugin_volume_mesh_from_surface
	LANGUAGES C CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(CGAL COMPONENTS Core)
if(CGAL_FOUND)
	include(${CGAL_USE_FILE})
	find_package(MPFR REQUIRED)
	find_package(GMP REQUIRED)
	find_package(Boost REQUIRED)
endif(CGAL_FOUND)

set(HEADER_FILES
	dll.h
	tetgen_structure_io.h
	volume_mesh_from_surface.h
	volume_mesh_from_surface_dialog.h
)

set(SOURCE_FILES
	tetgen_structure_io.cpp
	volume_mesh_from_surface.cpp
	volume_mesh_from_surface_dialog.cpp
)

if(CGAL_FOUND)
	set(HEADER_FILES
		${HEADER_FILES}
		cgal/cgogn_surface_to_cgal_polyhedron.h
		cgal/c3t3_import.h
		)
	set(SOURCE_FILES
		${SOURCE_FILES}
		cgal/cgogn_surface_to_cgal_polyhedron.cpp
		cgal/c3t3_import.cpp
		)
endif(CGAL_FOUND)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

if(CGAL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DPLUGIN_VMFS_WITH_CGAL")
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DBOOST_PARAMETER_MAX_ARITY=12")
endif(CGAL_FOUND)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	$<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${CGAL_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	tet
	plugin_image
	${CGAL_LIBRARIES}
	${cgogn_core_LIBRARIES}
	${cgogn_io_LIBRARIES}
	${CGAL_LIB}
	${Boost_LIBRARIES}
	${MPFR_LIBRARIES}
	${GMP_LIBRARIES}
)
