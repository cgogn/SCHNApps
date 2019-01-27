project(plugin_extract_surface
	LANGUAGES CXX
)


find_package(schnapps_core REQUIRED)
find_package(cgogn_io REQUIRED)

add_library(${PROJECT_NAME} SHARED "")
add_library(schnapps::extract_surface ALIAS plugin_extract_surface)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/extract_surface.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/extract_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/extract_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_EXTRACT_SURFACE_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps::core
	cgogn::io
	schnapps::cmap_provider
	cgogn::core
	Qt5::Widgets
	Qt5::Core
)
set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER "plugins")