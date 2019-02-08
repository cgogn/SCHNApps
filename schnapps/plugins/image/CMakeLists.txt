project(plugin_image
	LANGUAGES CXX
)

find_package(cgogn_io REQUIRED)
find_package(schnapps_core REQUIRED)
find_package(CGAL QUIET COMPONENTS Core ImageIO)

if(NOT CGAL_FOUND)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
endif()

if(MSVC)
	find_package(Boost COMPONENTS system iostreams zlib)
else()
	find_package(Boost COMPONENTS system iostreams)
endif()

find_package(CGAL COMPONENTS Core ImageIO)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::image ALIAS plugin_image)

if(CGAL_FOUND)
	include( ${CGAL_USE_FILE})
	if(MSVC)
		find_package(Boost QUIET REQUIRED COMPONENTS system thread iostreams zlib)
	else()
		find_package(Boost QUIET REQUIRED COMPONENTS system thread iostreams)
	endif()
	find_package(MPFR REQUIRED)
	find_package(GMP REQUIRED)
endif()


target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/image.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/image_dock_tab.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/image.ui"
		"${CMAKE_CURRENT_LIST_DIR}/image.h"
)
if (CGAL_FOUND)
	target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/cgal/cgal_image.cpp")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d" EXPORT_NAME image)


target_compile_definitions(${PROJECT_NAME} PRIVATE SCHNAPPS_PLUGIN_IMAGE_DLL_EXPORT)
if (CGAL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_IMAGE_WITH_CGAL)
endif()
if (Boost_IOSTREAMS_FOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM=1)
endif()


if(CGAL_FOUND AND WIN32)
	link_directories(${CGAL_LIBRARIES_DIR})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
)

if (CGAL_FOUND)
	target_include_directories(${PROJECT_NAME}  PUBLIC ${CGAL_INCLUDE_DIRS})
endif()

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	schnapps::cmap_provider
	cgogn::core
	cgogn::io
	Qt5::Widgets
	cgogn::QOGLViewer
	${CGAL_LIBRARIES}
)

if (CGAL_FOUND)
	target_link_libraries(${PROJECT_NAME} Boost::system Boost::thread)
endif()
if (Boost_IOSTREAMS_FOUND)
	target_link_libraries(${PROJECT_NAME} Boost::iostreams)
	if (WIN32)
		target_link_libraries(${PROJECT_NAME} Boost::zlib)
	endif()
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/image
	FILES_MATCHING PATTERN "*.h"
)

schnapps_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)

#In this section we copy boost's and CGAL's DLLs in schnapp's directory
if(WIN32)
	if(CGAL_FOUND)
		# ## Copy DLLs to the build tree
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			foreach(CGAL_LIB ${CGAL_LIBRARIES})
				get_target_property(LIB_DEBUG ${CGAL_LIB} IMPORTED_LOCATION_DEBUG)
				if (LIB_DEBUG AND EXISTS ${LIB_DEBUG})
					file(COPY ${LIB_DEBUG} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
				endif()
				get_target_property(LIB_RELEASE ${CGAL_LIB} IMPORTED_LOCATION_RELEASE)
				if (LIB_RELEASE AND EXISTS ${LIB_RELEASE})
					file(COPY ${LIB_RELEASE} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
				endif()
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


