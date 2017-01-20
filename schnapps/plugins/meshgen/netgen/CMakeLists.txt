cmake_minimum_required(VERSION 3.0)
project(netgen)

find_package(Zlib REQUIRED NO_CMAKE_ENVIRONMENT_PATH)

set(NETGEN_SRC
	libsrc/general/array.cpp
	libsrc/general/bitarray.cpp
	libsrc/general/hashtabl.cpp
	libsrc/general/symbolta.cpp
	libsrc/general/table.cpp
	libsrc/general/flags.cpp
	libsrc/general/spbita2d.cpp
	libsrc/general/seti.cpp
	libsrc/general/optmem.cpp
	libsrc/general/sort.cpp
	libsrc/general/mystring.cpp
	libsrc/general/parthreads.cpp
	libsrc/general/dynamicmem.cpp
	libsrc/general/ngexception.cpp
	libsrc/general/profiler.cpp
	libsrc/general/gzstream.cpp
	libsrc/linalg/densemat.cpp
	libsrc/linalg/polynomial.cpp
	libsrc/linalg/bfgs.cpp
	libsrc/linalg/linopt.cpp
	libsrc/linalg/linsearch.cpp
	libsrc/gprim/geom2d.cpp
	libsrc/gprim/geom3d.cpp
	libsrc/gprim/geomtest3d.cpp
	libsrc/gprim/adtree.cpp
	libsrc/gprim/transform3d.cpp
	libsrc/gprim/spline.cpp
	libsrc/gprim/geomfuncs.cpp
	libsrc/meshing/meshclass.cpp
	libsrc/meshing/adfront2.cpp
	libsrc/meshing/adfront3.cpp
	libsrc/meshing/geomsearch.cpp
	libsrc/meshing/global.cpp
	libsrc/meshing/meshtool.cpp
	libsrc/meshing/netrule2.cpp
	libsrc/meshing/netrule3.cpp
	libsrc/meshing/parser2.cpp
	libsrc/meshing/parser3.cpp
	libsrc/meshing/ruler2.cpp
	libsrc/meshing/ruler3.cpp
	libsrc/meshing/meshtype.cpp
	libsrc/meshing/improve2.cpp
	libsrc/meshing/smoothing2.5.cpp
	libsrc/meshing/smoothing2.cpp
	libsrc/meshing/improve3.cpp
	libsrc/meshing/smoothing3.cpp
	libsrc/meshing/improve2gen.cpp
	libsrc/meshing/meshing2.cpp
	libsrc/meshing/meshing3.cpp
	libsrc/meshing/localh.cpp
	libsrc/meshing/delaunay.cpp
	libsrc/meshing/topology.cpp
	libsrc/meshing/clusters.cpp
	libsrc/meshing/tetrarls.cpp
	libsrc/meshing/triarls.cpp
	libsrc/meshing/quadrls.cpp
	libsrc/meshing/meshfunc.cpp
	libsrc/meshing/refine.cpp
	libsrc/meshing/bisect.cpp
	libsrc/meshing/boundarylayer.cpp
	libsrc/meshing/specials.cpp
	libsrc/meshing/msghandler.cpp
	libsrc/meshing/pyramidrls.cpp
	libsrc/meshing/pyramid2rls.cpp
	libsrc/meshing/prism2rls.cpp
	libsrc/meshing/curvedelems.cpp
	libsrc/meshing/hexarls.cpp
	libsrc/meshing/validate.cpp
	libsrc/meshing/basegeom.cpp
	libsrc/stlgeom/stlgeom.cpp
	libsrc/stlgeom/stltool.cpp
	libsrc/stlgeom/stltopology.cpp
	libsrc/stlgeom/stlline.cpp
	libsrc/stlgeom/meshstlsurface.cpp
	libsrc/interface/nginterface.cpp
	libsrc/interface/writeuser.cpp
	libsrc/csg/csgeom.cpp
	libsrc/csg/surface.cpp
	libsrc/csg/solid.cpp
	libsrc/csg/revolution.cpp
	libsrc/csg/extrusion.cpp
	libsrc/csg/polyhedra.cpp
	libsrc/csg/algprim.cpp
	libsrc/geom2d/genmesh2d.cpp
	libsrc/geom2d/geometry2d.cpp
	)

add_library(${PROJECT_NAME} SHARED ${NETGEN_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libsrc/include>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_ZLIB_INCLUDE_DIR}>)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} ${Zlib_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DNGINTERFACE_EXPORTS")

add_subdirectory(nglib)
