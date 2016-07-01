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

#include "../dll.h"
#include <schnapps/core/map_handler.h>
#include <volume_mesh_from_surface.h>
#include <cgogn/io/volume_import.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/refine_mesh_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Image_3.h>
#include <CGAL/make_mesh_3.h>

namespace schnapps
{

namespace plugin_image
{
class Image3D;
}

namespace plugin_vmfs
{

template<typename C3T3>
class C3T3VolumeImport : public cgogn::io::VolumeImport<CMap3::MapTraits>
{
public:
	using Inherit = VolumeImport<CMap3::MapTraits>;
	using Self = C3T3VolumeImport<C3T3>;

	inline C3T3VolumeImport(const C3T3& cpx) : Inherit(),
		cpx_(cpx)
	{}
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(C3T3VolumeImport);

	template<typename T>
	using ChunkArray = Inherit::ChunkArray<T>;

	using Triangulation = typename C3T3::Triangulation;
	using Vertex_handle = typename Triangulation::Vertex_handle;

protected:
	virtual bool import_file_impl(const std::string& /*filename*/) override
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
private:
	const C3T3& cpx_;
};

template<typename C3T3>
void import_c3t3(const C3T3& c3t3_in, MapHandler<CMap3>* map_out)
{
	if (!map_out)
		return;

	C3T3VolumeImport<C3T3> volume_import(c3t3_in);
	volume_import.import_file("");
	volume_import.create_map(*map_out->get_map());
}


SCHNAPPS_PLUGIN_VMFS_API void tetrahedralize(const MeshGeneratorParameters& param, MapHandler<CMap2>* input_surface_map, const std::string& pos_att_name, MapHandler<CMap3>* output_volume_map);
SCHNAPPS_PLUGIN_VMFS_API void tetrahedralize(const MeshGeneratorParameters& param, const plugin_image::Image3D* im, MapHandler<CMap3>* output_volume_map);

template<typename Domain_>
void tetrahedralize(const MeshGeneratorParameters& param, Domain_& dom, CGAL::Mesh_criteria_3<typename CGAL::Mesh_triangulation_3<Domain_>::type>& criteria, MapHandler<CMap3>* output_volume_map)
{
	using namespace CGAL::parameters;
	using Triangulation_ = typename CGAL::Mesh_triangulation_3<Domain_>::type;
	using C3T3_ = typename CGAL::Mesh_complex_3_in_triangulation_3<Triangulation_>;

	auto c3t3 = CGAL::make_mesh_3<C3T3_>(dom, criteria, no_features(), no_perturb(), no_exude(), no_lloyd(), no_odt());

	if (param.do_lloyd_)
	{
		CGAL::lloyd_optimize_mesh_3(c3t3,
									dom,
									time_limit = 0,
									max_iteration_number = param.lloyd_max_iter_,
									convergence = param.lloyd_convergence_,
									freeze_bound = param.lloyd_freeze_bound_,
									do_freeze = param.do_lloyd_freeze_);
	}

	if (param.do_odt_)
	{
		CGAL::odt_optimize_mesh_3(c3t3,
									dom,
									time_limit = 0,
									max_iteration_number = param.odt_max_iter_,
									convergence = param.odt_convergence_,
									freeze_bound = param.odt_freeze_bound_,
									do_freeze = param.do_odt_freeze_);
	}

	if (param.do_perturber_)
	{
		CGAL::perturb_mesh_3(c3t3,
							dom,
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

} // namespace plugin_vmfs
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_C3T3_IMPORT_H
