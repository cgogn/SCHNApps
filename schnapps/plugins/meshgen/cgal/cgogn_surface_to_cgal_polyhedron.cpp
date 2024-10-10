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

#include "cgogn_surface_to_cgal_polyhedron.h"

#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

namespace schnapps
{

namespace plugin_meshgen
{

PolyhedronBuilder::PolyhedronBuilder(CMap2Handler* mh, const CMap2::VertexAttribute<VEC3>& position_attribute) :
	map_(mh),
	position_attribute_(position_attribute)
{}

void PolyhedronBuilder::operator()(HalfedgeDS& hds)
{
	if (!map_ || !position_attribute_.is_valid())
		return;

	uint32 id{0u};
	auto id_attribute = map_->add_attribute<uint32, CMap2::Vertex::ORBIT>("ids_polyhedron_builder");

	const CMap2& cmap2 = *map_->map();
	cmap2.foreach_cell([&](CMap2::Vertex v) { id_attribute[v] = id++; });

	CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> B( hds, true);
	B.begin_surface( cmap2.nb_cells<CMap2::Vertex::ORBIT>(), cmap2.nb_cells<CMap2::Face::ORBIT>() );

	cmap2.foreach_cell([&](CMap2::Vertex v)
	{
		const auto& P = position_attribute_[v];
		B.add_vertex((Point(P[0], P[1], P[2])));
	});

	cmap2.foreach_cell([&](CMap2::Face f)
	{
		B.begin_facet();
		cmap2.foreach_incident_vertex(f, [&](CMap2::Vertex v)
		{
			B.add_vertex_to_facet(id_attribute[v]);
		});
		B.end_facet();
	});

	B.end_surface();

	map_->remove_attribute(id_attribute);
}

SCHNAPPS_PLUGIN_MESHGEN_API std::unique_ptr<CGAL::Polyhedron_3<CGAL::Exact_predicates_inexact_constructions_kernel>> build_polyhedron(plugin_cmap_provider::CMap2Handler* mh, const CMap2::VertexAttribute<VEC3>& position_attribute)
{
	if (!mh || !position_attribute.is_valid())
		return nullptr;
	auto poly = cgogn::make_unique<CGAL::Polyhedron_3< CGAL::Exact_predicates_inexact_constructions_kernel>>();
	PolyhedronBuilder builder(mh, position_attribute);
	poly->delegate(builder);
	return poly;
}

} // namespace plugin_meshgen

} // namespace schnapps
