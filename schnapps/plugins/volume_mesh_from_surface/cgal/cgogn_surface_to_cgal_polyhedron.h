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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H

#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>
#include "types.h"

namespace schnapps
{

class PolyhedronBuilder : public CGAL::Modifier_base<HalfedgeDS> {
public:

	using Vertex = typename HalfedgeDS::Vertex;
	using Point = typename Vertex::Point ;

	PolyhedronBuilder(MapHandler<CMap2>* mh, std::string position_att_name);
	void operator()( HalfedgeDS& hds);
private:
	MapHandler<CMap2>* map_;
	std::string pos_att_name_;
};

std::unique_ptr<Polyhedron> build_polyhedron(MapHandler<CMap2>* mh, const std::string& position_att_name);

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_CGOGN_SURFACE_TO_CGAL_POLYHEDRON_H
