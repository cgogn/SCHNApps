project(schnapps_plugin_import
	LANGUAGES CXX
)

set(HEADER_FILES
	include/import.h
)

set(SOURCE_FILES
	src/import.cpp
)

set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

# use of target_compile_options to have a transitive c++11 flag
if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++11")
endif()
if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-D_USE_MATH_DEFINES")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(${PROJECT_NAME} PUBLIC
	include
	$<BUILD_INTERFACE:${SCHNAPPS_THIRDPARTY_QOGLVIEWER_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_EIGEN3_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_TINYXML2_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_LM6_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_PLY_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${CGOGN_PATH}/../CGoGN_2-build/lib/libcgogn_core.dylib
	${CGOGN_PATH}/../CGoGN_2-build/lib/libcgogn_io.dylib
	Qt5::Widgets
)
