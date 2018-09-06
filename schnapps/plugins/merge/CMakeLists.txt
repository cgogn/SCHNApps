project(plugin_merge
	LANGUAGES CXX
)

find_package(cgogn_geometry REQUIRED) # for Eigen

add_library(${PROJECT_NAME} SHARED "")

add_library(schnapps::merge ALIAS plugin_merge)

target_sources(${PROJECT_NAME}
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/dll.h"
		"${CMAKE_CURRENT_LIST_DIR}/merge.h"
		"${CMAKE_CURRENT_LIST_DIR}/merge.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.h"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/merge_dialog.ui"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
										DEBUG_POSTFIX "_d"
										EXPORT_NAME merge)

target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSCHNAPPS_PLUGIN_MERGE_PLUGIN_DLL_EXPORT")

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${SCHNAPPS_SOURCE_DIR}>
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
	schnapps_core
	cgogn_core
	cgogn_geometry
	Qt5::Core
	Qt5::Widgets
)

install(TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}Targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/schnapps/plugins/merge
	FILES_MATCHING PATTERN "*.h"
)

set_target_properties (${PROJECT_NAME} PROPERTIES FOLDER plugins)
