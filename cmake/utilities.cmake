
include(CMakePackageConfigHelpers)

macro(schnapps_create_package package_root_dir)

######## 1. Build tree

export(TARGETS ${PROJECT_NAME}
	NAMESPACE schnapps::
	FILE "${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake"
)

configure_package_config_file(
	"${package_root_dir}/${PROJECT_NAME}Config.cmake.in"
	"${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" 
	INSTALL_DESTINATION "${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}"
	NO_SET_AND_CHECK_MACRO
	NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
	"${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${SCHNAPPS_VERSION_MAJOR}.${SCHNAPPS_VERSION_MINOR}.${SCHNAPPS_VERSION_PATCH}
	COMPATIBILITY ExactVersion
)

######## 2. Install tree

install(TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}Targets
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT ${PROJECT_NAME}_applications
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT ${PROJECT_NAME}_libraries
        ARCHIVE
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT ${PROJECT_NAME}_libraries
)

install(EXPORT ${PROJECT_NAME}Targets
		NAMESPACE schnapps::
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
                COMPONENT ${PROJECT_NAME}_libraries
)

## <package_name>Config.cmake
configure_package_config_file(
        "${package_root_dir}/${PROJECT_NAME}Config.cmake.in"
        "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        COMPONENT ${PROJECT_NAME}_libraries
)

## <package_name>ConfigVersion.cmake
write_basic_package_version_file(
	"${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
	VERSION ${SCHNAPPS_VERSION_MAJOR}.${SCHNAPPS_VERSION_MINOR}.${SCHNAPPS_VERSION_PATCH}
	COMPATIBILITY ExactVersion
)

install(FILES "${CMAKE_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    COMPONENT ${PROJECT_NAME}_libraries
)

endmacro()

function(schnapps_list_subdirectory result current_directory)
  file(GLOB children RELATIVE ${current_directory} ${current_directory}/*)
  set(dirlist "")
  foreach(child ${children})
	if(IS_DIRECTORY ${current_directory}/${child})
		list(APPEND dirlist ${child})
	endif()
  endforeach()
  set(${result} ${dirlist} PARENT_SCOPE)
endfunction()
