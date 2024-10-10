﻿/*******************************************************************************
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

#include <schnapps/plugins/meshgen/meshgen.h>
#include <schnapps/plugins/meshgen/netgen_structure_io.h>

#include <netgen/libsrc/meshing/meshing.hpp>
#include <netgen/nglib/nglib.h>

namespace schnapps
{

namespace plugin_meshgen
{

std::unique_ptr<void*, std::function<void (void**)> > export_netgen(CMap2& map, const CMap2::VertexAttribute<VEC3>& pos)
{
	using Vertex = CMap2::Vertex;
	using Face = CMap2::Face;

	nglib::Ng_Init();
	std::unique_ptr<void*, std::function<void (void**)>> res(nglib::Ng_NewMesh(), &nglib::Ng_DeleteMesh);

	auto id_att = map.add_attribute<uint32, Vertex>("__netgen_points_ids_tmp__");
	uint32 it = 1u;
	map.foreach_cell([&it, &res, &id_att, &pos](Vertex v)
	{
		id_att[v] = it++;
		const auto p = pos[v];
		std::array<double,3> points = {double(p[0]), double(p[1]), double(p[2])};
		nglib::Ng_AddPoint(res.get(), &points[0]);
	});

	map.foreach_cell([&map,&res,&id_att](Face f)
	{
		std::array<int,3> tri_indices;
		uint32 i = 0u;
		map.foreach_incident_vertex(f, [&](Vertex v)
		{
			tri_indices[i++] = id_att[v];
		});
		nglib::Ng_AddSurfaceElement(res.get(), nglib::Ng_Surface_Element_Type::NG_TRIG, &tri_indices[0]);
	});

	map.remove_attribute(id_att);
	return res;
}

NetgenStructureVolumeImport::NetgenStructureVolumeImport(void** netgen_data, CMap3& map) :
	Inherit(map),
	volume_mesh_structure_(netgen_data)
{
	import_netgen_structure();
}

void NetgenStructureVolumeImport::import_netgen_structure()
{
	ChunkArray<VEC3>* position = this->add_vertex_attribute<VEC3>("position");

	const int nb_points = nglib::Ng_GetNP(volume_mesh_structure_);
	for (int i = 1 ; i <= nb_points; ++i)
	{
		std::array<double, 3> p;
		nglib::Ng_GetPoint(volume_mesh_structure_,i, &p[0]);
		const unsigned id = this->insert_line_vertex_container();
		position->operator[](id) = VEC3(Scalar(p[0]), Scalar(p[1]), Scalar(p[2]));
	}

	const int nb_vol = nglib::Ng_GetNE(volume_mesh_structure_);
	for (int i = 1 ; i <= nb_vol; ++i)
	{
		std::array<int, 4> tetra;
		nglib::Ng_GetVolumeElement(volume_mesh_structure_, i, &tetra[0]);
		std::array<uint32, 4> ids;
		for (uint32 j = 0u; j < 4u; j++)
			ids[j] = tetra[j] - 1;
		this->reorient_tetra(*position, ids[0], ids[1], ids[2], ids[3]);
		this->add_tetra(ids[0], ids[1], ids[2], ids[3]);
	}
}

nglib::Ng_Meshing_Parameters* setup_netgen_parameters(const NetgenParameters& params)
{
	nglib::Ng_Meshing_Parameters* mp = new nglib::Ng_Meshing_Parameters();
	mp->uselocalh = params.uselocalh;
	mp->maxh = params.maxh;
	mp->minh = params.minh;
	mp->fineness = params.fineness;
	mp->grading = params.grading;
	mp->elementsperedge = params.elementsperedge;
	mp->elementspercurve = params.elementspercurve;
	mp->closeedgeenable = params.closeedgeenable;
	mp->minedgelen = params.minedgelen;
	mp->second_order = params.second_order;
	mp->quad_dominated = params.quad_dominated;
	mp->meshsize_filename = params.meshsize_filename;
	mp->optsurfmeshenable = params.optsurfmeshenable;
	mp->optvolmeshenable = params.optvolmeshenable;
	mp->optsteps_2d = params.optsteps_2d;
	mp->optsteps_3d = params.optsteps_3d;
	mp->invert_tets = params.invert_tets;
	mp->invert_trigs = params.invert_trigs;
	mp->check_overlap = params.check_overlap;
	mp->check_overlapping_boundary = params.check_overlapping_boundary;
	return mp;
}

} // namespace plugin_meshgen
} // namespace schnapps
