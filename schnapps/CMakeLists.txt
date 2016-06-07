add_subdirectory(core)
add_subdirectory(plugins)

project(schnapps
	LANGUAGES CXX
)

find_package(Qt5 5.4.0 COMPONENTS Widgets REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	${Qt5Widgets_LIBRARIES}
)
