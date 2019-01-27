# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 3.0)

project(tet
	LANGUAGES CXX
)

if(MSVC)
	add_definitions("-D_CRT_SECURE_NO_WARNINGS /W0")
endif(MSVC)

add_library(tet STATIC tetgen.h tetgen.cxx predicates.cxx)

if(NOT MSVC)
	target_compile_options(tet PUBLIC "-std=c++11")
endif(NOT MSVC)
set_property(TARGET tet PROPERTY POSITION_INDEPENDENT_CODE TRUE)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "plugins/meshgen")