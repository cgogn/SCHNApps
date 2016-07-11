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
#include <cgal/cgal_image.h>
#include <CGAL/Labeled_image_mesh_domain_3.h>
#include <CGAL/Polyhedral_mesh_domain_3.h>

namespace schnapps
{

namespace plugin_vmfs
{

SCHNAPPS_PLUGIN_VMFS_API void tetrahedralize(const MeshGeneratorParameters& param, MapHandler<CMap2>* input_surface_map, const CMap2::VertexAttribute<VEC3>& position_attribute, MapHandler<CMap3>* output_volume_map)
{

	using Kernel		= CGAL::Exact_predicates_inexact_constructions_kernel;
	using Polyhedron	= CGAL::Polyhedron_3<Kernel>;
	using Domain		= CGAL::Polyhedral_mesh_domain_3<Polyhedron, Kernel>;
	using Triangulation	= CGAL::Mesh_triangulation_3<Domain>::type;
	using Criteria		= CGAL::Mesh_criteria_3<Triangulation>;
	using C3T3			= CGAL::Mesh_complex_3_in_triangulation_3<Triangulation>;
	using namespace CGAL::parameters;

	if (!input_surface_map || !output_volume_map || !position_attribute.is_valid())
		return;

	auto poly = build_polyhedron(input_surface_map, position_attribute);
	if (poly)
	{

		Criteria criteria(
					cell_size = param.cell_size_,
					facet_angle = param.facet_angle_,
					facet_size =  param.facet_size_,
					facet_distance= param.facet_distance_,
					cell_radius_edge_ratio = param.cell_radius_edge_ratio_);

		Domain mesh_domain(*poly);

		tetrahedralize(param, mesh_domain, criteria, output_volume_map);
	}
}

SCHNAPPS_PLUGIN_VMFS_API void tetrahedralize(const MeshGeneratorParameters& param, const plugin_image::Image3D* im, MapHandler<CMap3>* output_volume_map)
{
	using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
#if CGAL_VERSION_NR >= CGAL_VERSION_NUMBER(4,8,0)
	using Mesh_domain = CGAL::Labeled_image_mesh_domain_3<CGAL::Image_3, Kernel, plugin_image::Image3D::value_type, float32>;
#else
	using Mesh_domain = CGAL::Labeled_image_mesh_domain_3<CGAL::Image_3, Kernel>;
#endif
	using Tr = CGAL::Mesh_triangulation_3<Mesh_domain>::type;
	using C3t3 = CGAL::Mesh_complex_3_in_triangulation_3<Tr>;
	using Mesh_criteria = CGAL::Mesh_criteria_3<Tr>;
	using namespace CGAL::parameters;

	if (!im || !output_volume_map)
		return;

	//if (im->get_data_type() != cgogn::io::DataType::FLOAT)
	//{
	//	std::cerr << "Error : CGAL::Labeled_image_mesh_domain_3 requires labels to be of type float32." << std::endl;
	//	return;
	//}

	Mesh_criteria criteria(
				cell_size = param.cell_size_,
				facet_angle = param.facet_angle_,
				facet_size =  param.facet_size_,
				facet_distance= param.facet_distance_,
				cell_radius_edge_ratio = param.cell_radius_edge_ratio_);

	auto cgal_im = plugin_image::export_to_cgal_image(*im);

	Mesh_domain domain(cgal_im);

	tetrahedralize(param, domain, criteria, output_volume_map);
}

} // namespace plugin_vmfs
} // namespace schnapps
