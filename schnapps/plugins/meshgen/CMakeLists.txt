add_subdirectory(tetgen)

project(plugin_meshgen
	LANGUAGES C CXX
)

find_package(cgogn_core REQUIRED)
find_package(cgogn_io REQUIRED)
find_package(cgogn_modeling REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(CGAL COMPONENTS Core ImageIO)

if(NOT CGAL_FOUND)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
	find_package(CGAL COMPONENTS Core ImageIO)
endif()

if(CGAL_FOUND)
	include(${CGAL_USE_FILE})
	find_package(MPFR REQUIRED)
	find_package(GMP REQUIRED)
	find_package(Boost REQUIRED)
endif(CGAL_FOUND)

add_subdirectory(netgen)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::meshgen ALIAS plugin_meshgen)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_MESHGEN_DLL_EXPORT")

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/netgen_structure_io.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/tetgen_structure_io.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/meshgen.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/meshgen_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/netgen_export.ui"
		"${CMAKE_CURRENT_LIST_DIR}/tetgen_export.ui"
		$<$<BOOL:CGAL_FOUND>:${CMAKE_CURRENT_LIST_DIR}/cgal/cgogn_surface_to_cgal_polyhedron.cpp>
		$<$<BOOL:CGAL_FOUND>:${CMAKE_CURRENT_LIST_DIR}/cgal/c3t3_import.cpp>
		$<$<BOOL:CGAL_FOUND>:${CMAKE_CURRENT_LIST_DIR}/cgal_export.ui>
)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		$<$<BOOL:CGAL_FOUND>:PLUGIN_MESHGEN_WITH_CGAL>
		$<$<BOOL:WITH_CGAL_ImageIO>:PLUGIN_MESHGEN_WITH_CGAL_IMAGEIO>
		$<$<BOOL:CGAL_FOUND>:BOOST_PARAMETER_MAX_ARITY=12>
)

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
	schnapps::cmap_provider
	cgogn::io
	netgen
	tet
	schnapps::image
	${CGAL_LIBRARIES}
	cgogn::core
	cgogn::modeling
	${Boost_LIBRARIES}
	${MPFR_LIBRARIES}
	${GMP_LIBRARIES}
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "plugins/meshgen")

#In this section we copy boost's and CGAL's DLLs in schnapp's directory
if(WIN32)
	if(CGAL_FOUND)
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			foreach(CGAL_LIB ${CGAL_LIBRARIES})
				get_target_property(LIB_DEBUG ${CGAL_LIB} IMPORTED_LOCATION_DEBUG)
				get_target_property(LIB_RELEASE ${CGAL_LIB} IMPORTED_LOCATION_RELEASE)
				file(COPY ${LIB_DEBUG} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
				file(COPY ${LIB_RELEASE} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
			endforeach()
		endforeach()
	endif()
	if(Boost_FOUND)
		#BOOST_ALL_DYN_LINK: Forces all libraries that have separate source, 
		#to be linked as DLL's rather than static libraries on Microsoft Windows
		target_compile_definitions(${PROJECT_NAME} PRIVATE "-DBOOST_ALL_DYN_LINK")
		get_filename_component(BOOST_SYSTEM_PATH ${Boost_SYSTEM_LIBRARY_RELEASE} PATH)
		file(GLOB BOOST_DLLS "${BOOST_SYSTEM_PATH}/*.dll")
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			file(COPY ${BOOST_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
		endforeach()
	endif()
endif(WIN32)