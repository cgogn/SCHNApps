/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin MeshGen                                                               *
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

#ifndef SCHNAPPS_PLUGIN_MESHGEN_C3T3_IMPORT_H
#define SCHNAPPS_PLUGIN_MESHGEN_C3T3_IMPORT_H

#include <dll.h>
#include <meshgen.h>
#include <schnapps/core/map_handler.h>
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

namespace plugin_meshgen
{

template<typename C3T3>
class C3T3VolumeImport : public cgogn::io::VolumeImport<VEC3>
{
public:

	using Inherit = VolumeImport<VEC3>;
	using Self = C3T3VolumeImport<C3T3>;

	inline C3T3VolumeImport(const C3T3& cpx) : Inherit(), cpx_(cpx)
	{
		import_c3t3();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(C3T3VolumeImport);

	template<typename T>
	using ChunkArray = Inherit::ChunkArray<T>;

	using Triangulation = typename C3T3::Triangulation;
	using Vertex_handle = typename Triangulation::Vertex_handle;

	void import_c3t3()
	{
		const Triangulation& triangulation = cpx_.triangulation();
		std::map<Vertex_handle, unsigned int> vertices_indices;
		ChunkArray<VEC3>* position = this->position_attribute();

		const uint32 num_cells = cpx_.number_of_cells_in_complex();
		this->reserve(num_cells);

		for (auto vit = triangulation.finite_vertices_begin(), vend = triangulation.finite_vertices_end(); vit != vend; ++vit)
		{
			const auto& P = vit->point();
			const uint32 id = this->insert_line_vertex_container();
			vertices_indices[vit] = id;
			position->operator [](id) = VEC3(SCALAR(P.x()), SCALAR(P.y()), SCALAR(P.z()));
		}

		for (auto cit = cpx_.cells_in_complex_begin(), cend = cpx_.cells_in_complex_end(); cit != cend; ++cit)
			this->add_tetra(vertices_indices[cit->vertex(0)], vertices_indices[cit->vertex(1)], vertices_indices[cit->vertex(2)], vertices_indices[cit->vertex(3)], true);


		ChunkArray<float32>* subdomain_indices = this->template add_volume_attribute<float32>("subdomain index");
		for (auto cit = cpx_.cells_in_complex_begin(), cend = cpx_.cells_in_complex_end(); cit != cend; ++cit)
		{
			const uint32 id = this->insert_line_volume_container();
			subdomain_indices->operator [](id) = float32(cpx_.subdomain_index(cit));
		}
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
	volume_import.create_map(*map_out->get_map());
	map_out->attribute_added(CMap3::Vertex::ORBIT, "position");
	map_out->attribute_added(CMap3::Volume::ORBIT, "subdomain index");
	map_out->set_bb_vertex_attribute("position");
	static_cast<MapHandlerGen*>(map_out)->create_vbo("position");
}


SCHNAPPS_PLUGIN_MESHGEN_API void tetrahedralize(const CGALParameters& param, CMap2Handler* input_surface_map, const CMap2::VertexAttribute<VEC3>& position_attribute, CMap3Handler* output_volume_map);
SCHNAPPS_PLUGIN_MESHGEN_API void tetrahedralize(const CGALParameters& param, const plugin_image::Image3D* im, CMap3Handler* output_volume_map);

template<typename Domain_>
void tetrahedralize(const CGALParameters& param, Domain_& dom, CGAL::Mesh_criteria_3<typename CGAL::Mesh_triangulation_3<Domain_>::type>& criteria, CMap3Handler* output_volume_map)
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

} // namespace plugin_meshgen

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_MESHGEN_C3T3_IMPORT_H
