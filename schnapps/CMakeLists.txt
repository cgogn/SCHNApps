find_package(Qt5 5.4.0 COMPONENTS Widgets REQUIRED)
find_package(QOGLViewer REQUIRED)
find_package(cgogn_core REQUIRED)

#CMake will handle .qrc files added as target sources
set(CMAKE_AUTORCC ON)
#CMake will handle automatically moc for all Qt targets
set(CMAKE_AUTOMOC ON)
#CMake will handle automatically uic for all Qt targets
set(CMAKE_AUTOUIC ON)


include(GenerateExportHeader)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/core")

set(SCHNAPPS_SYSTEM_PLUGIN_PATH ${SCHNAPPS_SOURCE_DIR}/schnapps/plugins)

add_subdirectory(core)
add_subdirectory(plugins)

project(${SCHNAPPS_EXEC_NAME}
	LANGUAGES CXX
)

if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
	get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

	execute_process(
		COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
		RESULT_VARIABLE return_code
		OUTPUT_VARIABLE qt5_install_prefix
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

	if(EXISTS ${imported_location})
		add_executable(Qt5::windeployqt IMPORTED)

		set_target_properties(Qt5::windeployqt PROPERTIES
			IMPORTED_LOCATION ${imported_location}
		)
	endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${SCHNAPPS_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${cgogn_core_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME} schnapps_core Qt5::Widgets)

install(TARGETS ${SCHNAPPS_EXEC_NAME}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(TARGET Qt5::windeployqt)
	# execute windeployqt in a tmp directory after build
	add_custom_command(TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
		COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
		COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
	)

	# copy deployment directory during installation
	install(
		DIRECTORY
		"${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)

include(InstallRequiredSystemLibraries)

install(
	PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
