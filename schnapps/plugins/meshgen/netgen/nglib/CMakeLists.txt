cmake_minimum_required(VERSION 3.0)
project(nglib)

add_library(${PROJECT_NAME} SHARED nglib.cpp)

target_link_libraries(${PROJECT_NAME} netgen)
#${CMAKE_THREAD_LIBS_INIT}

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../libsrc/include>)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DNGLIB_EXPORTS")
