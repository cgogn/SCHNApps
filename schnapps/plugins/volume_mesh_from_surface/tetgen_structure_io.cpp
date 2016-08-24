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

#include <tetgen_structure_io.h>
#include <tetgen/tetgen.h>

namespace schnapps
{

namespace plugin_vmfs
{

SCHNAPPS_PLUGIN_VMFS_API std::unique_ptr<tetgen::tetgenio> export_tetgen(CMap2& map, const CMap2::VertexAttribute<VEC3>& pos)
{
	using Vertex = CMap2::Vertex;
	using Face = CMap2::Face;
	using tetgenio = tetgen::tetgenio;
	using TetgenReal = REAL;

	map.compact_embedding(Vertex::ORBIT);
	std::unique_ptr<tetgenio> output = cgogn::make_unique<tetgenio>();

	// 0-based indexing
	output->firstnumber = 0;

	// input vertices
	output->numberofpoints = map.template nb_cells<Vertex::ORBIT>();
	output->pointlist = new TetgenReal[output->numberofpoints * 3];

	//for each vertex

	map.foreach_cell([&output, &map, &pos] (Vertex v)
	{
		const VEC3& vec = pos[v];
		const uint32 emb = map.embedding(v);
		output->pointlist[3u*emb + 0u] = vec[0];
		output->pointlist[3u*emb + 1u] = vec[1];
		output->pointlist[3u*emb + 2u] = vec[2];
	});

	output->numberoffacets = map.template nb_cells<Face::ORBIT>();
	output->facetlist = new tetgenio::facet[output->numberoffacets] ;

	//for each facet
	uint32 i = 0u;
	map.foreach_cell([&output, &i, &map] (Face face)
	{
		tetgenio::facet* f = &(output->facetlist[i]);
		tetgenio::init(f);
		f->numberofpolygons = 1;
		f->polygonlist = new tetgenio::polygon[1];
		tetgenio::polygon* p = &f->polygonlist[0];
		tetgenio::init(p);
		p->numberofvertices = map.codegree(face);
		p->vertexlist = new int[p->numberofvertices];

		uint32 j = 0u;
		map.foreach_incident_vertex(face, [&p, &map, &j] (Vertex v)
		{
			p->vertexlist[j++] = map.embedding(v);
		});

		++i;
	});

	return output;
}

TetgenStructureVolumeImport::TetgenStructureVolumeImport(tetgenio* tetgen_output)
{
	volume_ = tetgen_output;
	import_tetgen_structure();
}

bool TetgenStructureVolumeImport::import_tetgen_structure()
{
	const uint32 nb_vertices = volume_->numberofpoints;
	const uint32 nb_volumes = volume_->numberoftetrahedra;
	this->reserve(nb_volumes);

	if (nb_vertices == 0u || nb_volumes== 0u)
	{
		cgogn_log_warning("TetgenStructureVolumeImport") << "Error while importing data.";
		this->clear();
		return false;
	}

	ChunkArray<VEC3>* position = this->template position_attribute<VEC3>();
	//create vertices
	std::vector<uint32> vertices_indices;
	float64* p = volume_->pointlist ;

	for(uint32 i = 0u; i < nb_vertices; ++i)
	{
		const unsigned id = this->insert_line_vertex_container();
		position->operator[](id) = VEC3(Scalar(p[0]), Scalar(p[1]), Scalar(p[2]));
		vertices_indices.push_back(id);
		p += 3 ;
	}

	//create tetrahedrons
	int* t = volume_->tetrahedronlist ;
	for(uint32 i = 0u; i < nb_volumes; ++i)
	{
		std::array<uint32,4> ids;
		for(uint32 j = 0u; j < 4u; j++)
			ids[j] = uint32(vertices_indices[t[j] - volume_->firstnumber]);
		this->add_tetra(*position, ids[0], ids[1], ids[2], ids[3], true);
		t += 4 ;
	}

	return true;
}

} // namespace plugin_vmfs
} // namespace schnapps
