add_subdirectory(core)
add_subdirectory(plugins)

project(schnapps_main
	LANGUAGES CXX
)

add_executable(schnapps main.cpp)
target_link_libraries(schnapps schnapps_core)
