
file(GLOB plugin_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(plug_dir ${plugin_dirs})
  if(IS_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/${plug_dir})
	  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${plug_dir}/CMakeLists.txt")
		  option(USE_PLUGIN_${plug_dir} "compile $(plug_dir}" ON )
	  endif()
  endif()
endforeach()


if ((Qt5Widgets_VERSION VERSION_GREATER 5.5.99) AND ${USE_PLUGIN_surface_render_transp})
  add_definitions("-D USE_TRANSPARENCY")
  set (USE_TRANSPARENCY ON)
endif()

foreach(plug_dir ${plugin_dirs})
  if(IS_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/${plug_dir})
	  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${plug_dir}/CMakeLists.txt")
		  if (${USE_PLUGIN_${plug_dir}})
			  add_subdirectory( ${plug_dir} )
			  target_compile_definitions(plugin_${plug_dir} PRIVATE -DSCHNAPPS_PLUGIN_NAME=QString\(\"${plug_dir}\"\))
		  endif()
	  endif()
  endif()
endforeach()

