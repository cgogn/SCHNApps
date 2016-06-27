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

#define SCHNAPPS_PLUGIN_VMFS_DLL_EXPORT

#include "c3t3_import.h"
#include "cgogn_surface_to_cgal_polyhedron.h"

#include <volume_mesh_from_surface.h>

#include <CGAL/make_mesh_3.h>

namespace schnapps
{

namespace plugin_vmfs
{

bool C3T3VolumeImport::import_file_impl(const std::string&)
{
	const Triangulation& triangulation = cpx_.triangulation();
	std::map<Vertex_handle, unsigned int> vertices_indices;
	ChunkArray<VEC3>* position = this->template position_attribute<VEC3>();

	const uint32 num_vertices = triangulation.number_of_vertices();
	const uint32 num_cells = cpx_.number_of_cells_in_complex();

	this->set_nb_volumes(num_cells);
	this->set_nb_vertices(num_vertices);

	for (auto vit = triangulation.finite_vertices_begin(), vend = triangulation.finite_vertices_end(); vit != vend; ++vit)
	{
		const auto& P = vit->point();
		const uint32 id = this->insert_line_vertex_container();
		vertices_indices[vit] = id;
		position->operator [](id) = VEC3(SCALAR(P.x()), SCALAR(P.y()), SCALAR(P.z()));
	}

	for (auto cit = cpx_.cells_in_complex_begin(), cend = cpx_.cells_in_complex_end(); cit != cend; ++cit)
		this->add_tetra(*position, vertices_indices[cit->vertex(0)], vertices_indices[cit->vertex(1)], vertices_indices[cit->vertex(2)], vertices_indices[cit->vertex(3)], true);

	ChunkArray<int32>* subdomain_indices = this->volume_attributes_container().template add_chunk_array<int32>("subdomain index");
	for (auto cit = cpx_.cells_in_complex_begin(), cend = cpx_.cells_in_complex_end(); cit != cend; ++cit)
	{
		const uint32 id = this->volume_attributes_container().template insert_lines<1>();
		subdomain_indices->operator [](id) = cpx_.subdomain_index(cit);
	}

	return true;
}

SCHNAPPS_PLUGIN_VMFS_API void import_c3t3(const C3T3& c3t3_in, MapHandler<CMap3>* map_out)
{
	if (!map_out)
		return;

	C3T3VolumeImport volume_import(c3t3_in);
	volume_import.import_file("");
	volume_import.create_map(*map_out->get_map());
}

SCHNAPPS_PLUGIN_VMFS_API void tetrahedralize(const MapParameters& param, MapHandler<CMap2>* input_surface_map, const std::string& pos_att_name, MapHandler<CMap3>* output_volume_map)
{
	using namespace CGAL::parameters;

	if (!input_surface_map || !output_volume_map || pos_att_name.empty())
		return;

	auto poly = build_polyhedron(input_surface_map, pos_att_name);
	if (poly)
	{
		const Criteria criteria(
					cell_size = param.cell_size_,
					facet_angle = param.facet_angle_,
					facet_size =  param.facet_size_,
					facet_distance= param.facet_distance_,
					cell_radius_edge_ratio = param.cell_radius_edge_ratio_);

		Domain mesh_domain(*poly);
		auto c3t3 = CGAL::make_mesh_3<C3T3>(mesh_domain, criteria, no_features(), no_perturb(), no_exude(), no_lloyd(), no_odt());

		if (param.do_lloyd_)
		{
			CGAL::lloyd_optimize_mesh_3(c3t3,
										mesh_domain,
										time_limit = 0,
										max_iteration_number = param.lloyd_max_iter_,
										convergence = param.lloyd_convergence_,
										freeze_bound = param.lloyd_freeze_bound_,
										do_freeze = param.do_lloyd_freeze_);
		}

		if (param.do_odt_)
		{
			CGAL::odt_optimize_mesh_3(c3t3,
										mesh_domain,
										time_limit = 0,
										max_iteration_number = param.odt_max_iter_,
										convergence = param.odt_convergence_,
										freeze_bound = param.odt_freeze_bound_,
										do_freeze = param.do_odt_freeze_);
		}

		if (param.do_perturber_)
		{
			CGAL::perturb_mesh_3(c3t3,
								mesh_domain,
								time_limit = 0,
								sliver_bound = param.perturber_sliver_bound_);
		}

		if (param.do_exuder_)
		{
			CGAL::exude_mesh_3(c3t3,
								time_limit = 0,
								sliver_bound = param.exuder_sliver_bound_);
		}

		import_c3t3(c3t3,output_volume_map);
	}
}

} // namespace plugin_vmfs
} // namespace schnapps
