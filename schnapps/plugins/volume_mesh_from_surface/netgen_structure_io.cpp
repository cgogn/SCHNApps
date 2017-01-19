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

#include <netgen_structure_io.h>
#include <netgen/libsrc/meshing/meshing.hpp>
#include <netgen/nglib/nglib.h>

namespace schnapps
{

namespace plugin_vmfs
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

NetgenStructureVolumeImport::NetgenStructureVolumeImport(void** netgen_data) : Inherit(),
	volume_mesh_structure_(netgen_data)
{
	import_netgen_structure();
}

bool NetgenStructureVolumeImport::import_netgen_structure()
{
	ChunkArray<VEC3>* position = this->position_attribute();

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
		nglib::Ng_GetVolumeElement(volume_mesh_structure_,i, &tetra[0]);
		this->add_tetra(tetra[0]-1, tetra[1]-1, tetra[2]-1, tetra[3]-1, true);
	}
}

} // namespace plugin_vmfs
} // namespace schnapps
