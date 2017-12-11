project(plugin_image
	LANGUAGES CXX
)

find_package(cgogn_io REQUIRED)

if(MSVC)
	find_package(Boost COMPONENTS system iostreams zlib)
else()
	find_package(Boost COMPONENTS system iostreams)
endif()
find_package(CGAL COMPONENTS Core ImageIO)

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

set(HEADER_FILES
	dll.h
	image.h
	image_dock_tab.h
)

set(SOURCE_FILES
	image.cpp
	image_dock_tab.cpp
)

if(CGAL_FOUND)
	set(HEADER_FILES
		${HEADER_FILES}
		cgal/cgal_image.h
		)
	set(SOURCE_FILES
		${SOURCE_FILES}
		cgal/cgal_image.cpp
		)
endif(CGAL_FOUND)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})


if(Boost_IOSTREAMS_FOUND)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM=1")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_IMAGE_DLL_EXPORT")
if(CGAL_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "-DPLUGIN_IMAGE_WITH_CGAL")
	if(WIN32)
		link_directories(${CGAL_LIBRARIES_DIR})
	endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

#target_compile_definitions(${PROJECT_NAME} PUBLIC "-Dcimg_display=0")

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${cgogn_core_LIBRARIES}
	${cgogn_io_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QOGLViewer_LIBRARIES}
)

if(CGAL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CGAL_INCLUDE_DIRS}>)
	target_link_libraries(${PROJECT_NAME} ${CGAL_LIBRARIES})
endif()

if(Boost_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()


#In this section we copy boost's and CGAL's DLLs in schnapp's directory
if(WIN32)
	if(CGAL_FOUND)
		file(GLOB CGAL_DLLS "${CGAL_INSTALL_PREFIX}/bin/*.dll")
		file(GLOB CGAL_AUX_DLLS "${GMP_INCLUDE_DIR}/../lib/*.dll")
		# # We suppose that the auxiliary DLLS for CGAL are always used from the CGAL package
		# ## Copy DLLs to the build tree
		foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
			file(COPY ${CGAL_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
			file(COPY ${CGAL_AUX_DLLS} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
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
