add_subdirectory(tetgen)

project(plugin_meshgen
	LANGUAGES C CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(cgogn_modeling REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(CGAL COMPONENTS Core ImageIO)
if(CGAL_FOUND)
	include(${CGAL_USE_FILE})
	find_package(MPFR REQUIRED)
	find_package(GMP REQUIRED)
	find_package(Boost REQUIRED)
endif(CGAL_FOUND)

add_subdirectory(netgen)

set(HEADER_FILES
	dll.h
	netgen_structure_io.h
	tetgen_structure_io.h
	meshgen.h
	meshgen_dialog.h
)

set(SOURCE_FILES
	netgen_structure_io.cpp
	tetgen_structure_io.cpp
	meshgen.cpp
	meshgen_dialog.cpp
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

#set(CMAKE_AUTOUIC ON)
# automoc bug  with cmake 3.0 ????
file(GLOB UIS "." "*.ui")
qt5_wrap_ui( UI_HEADERS ${UIS} )

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${UI_HEADERS})

if(CGAL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DPLUGIN_MESHGEN_WITH_CGAL")
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DBOOST_PARAMETER_MAX_ARITY=12")
	target_link_libraries(${PROJECT_NAME} plugin_image ${CGAL_ImageIO_LIBRARY})
endif(CGAL_FOUND)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_MESHGEN_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${CGAL_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/netgen/libsrc/include>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap0_provider
	schnapps::cmap1_provider
	schnapps::cmap2_provider
	schnapps::cmap3_provider
	cgogn::io
	netgen
	tet
	plugin_image
	${CGAL_LIBRARIES}
	cgogn::core
	cgogn::modeling
	${CGAL_Core_LIBRARY}
	${Boost_LIBRARIES}
	${MPFR_LIBRARIES}
	${GMP_LIBRARIES}
)

if(CGAL_FOUND AND WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CGAL_INSTALL_PREFIX}/bin"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
