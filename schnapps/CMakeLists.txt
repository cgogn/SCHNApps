add_subdirectory(core)
add_subdirectory(plugins)

project(schnapps
	LANGUAGES CXX
)

find_package(Qt5 5.4.0 COMPONENTS Widgets REQUIRED)
find_package(cgogn_core REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

# with Qt >5.7 & CMake < 3.1 need c++11 option
if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "-std=c++11")
endif(NOT MSVC)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${Qt5Widgets_LIBRARIES}
)
