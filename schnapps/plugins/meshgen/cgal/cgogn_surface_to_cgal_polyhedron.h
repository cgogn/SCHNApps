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

#ifndef SCHNAPPS_PLUGIN_MESHGEN_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H
#define SCHNAPPS_PLUGIN_MESHGEN_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H

#include "dll.h"
#include <schnapps/core/types.h>
#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>

namespace schnapps
{

namespace plugin_meshgen
{

class SCHNAPPS_PLUGIN_MESHGEN_API PolyhedronBuilder : public CGAL::Modifier_base<CGAL::Polyhedron_3<CGAL::Exact_predicates_inexact_constructions_kernel>::HalfedgeDS>
{
public:

	using Kernel =  CGAL::Exact_predicates_inexact_constructions_kernel;
	using Polyhedron = CGAL::Polyhedron_3<Kernel> ;
	using HalfedgeDS = Polyhedron::HalfedgeDS;
	using Vertex = HalfedgeDS::Vertex;
	using Point = Vertex::Point ;
	using CMap2Handler = plugin_cmap2_provider::CMap2Handler;

	PolyhedronBuilder(CMap2Handler* mh, const CMap2::VertexAttribute<VEC3>& position_attribute);
	void operator()(HalfedgeDS& hds);

private:

	CMap2Handler* map_;
	const CMap2::VertexAttribute<VEC3> position_attribute_;
};

SCHNAPPS_PLUGIN_MESHGEN_API std::unique_ptr<CGAL::Polyhedron_3<CGAL::Exact_predicates_inexact_constructions_kernel>> build_polyhedron(plugin_cmap2_provider::CMap2Handler* mh, const CMap2::VertexAttribute<VEC3>& position_attribute);

} // namespace plugin_meshgen

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_MESHGEN_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H
