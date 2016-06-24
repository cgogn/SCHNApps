/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Mesh From Surface                                              *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_C3T3_IMPORT_H
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_C3T3_IMPORT_H

#include "types.h"

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/refine_mesh_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>

#include <schnapps/core/map_handler.h>

#include <cgogn/io/volume_import.h>

namespace schnapps
{

// forward declaration of MapParameters
class MapParameters;

using Domain		= CGAL::Polyhedral_mesh_domain_3<Polyhedron, Kernel>;
using Triangulation	= CGAL::Mesh_triangulation_3<Domain>::type;
using Criteria		= CGAL::Mesh_criteria_3<Triangulation>;
using C3T3			= CGAL::Mesh_complex_3_in_triangulation_3<Triangulation>;


class C3T3VolumeImport : public cgogn::io::VolumeImport<CMap3::MapTraits>
{
public:
	using Inherit = VolumeImport<CMap3::MapTraits>;
	using Self = C3T3VolumeImport;

	inline C3T3VolumeImport(const C3T3& cpx) : Inherit(),
		cpx_(cpx)
	{}
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(C3T3VolumeImport);

	template<typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	using Triangulation = typename C3T3::Triangulation;
	using Vertex_handle = typename Triangulation::Vertex_handle;

protected:
	virtual bool import_file_impl(const std::string& /*filename*/) override;
private:
	const C3T3& cpx_;
};

void import_c3t3(const C3T3& c3t3_in, MapHandler<CMap3>* map_out);
void tetrahedralize(const MapParameters& param, MapHandler<CMap2>* input_surface_map, const std::string& pos_att_name, MapHandler<CMap3>* output_volume_map);

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_C3T3_IMPORT_H
