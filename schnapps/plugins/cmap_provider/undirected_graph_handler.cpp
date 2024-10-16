/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
*                                                                              *
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

#include <schnapps/plugins/cmap_provider/undirected_graph_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>
#include <schnapps/core/view.h>

#include <cgogn/rendering/shaders/vbo.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

UndirectedGraphHandler::UndirectedGraphHandler(const QString& name, PluginProvider* p) :
	CMapHandlerGen(name, p)
{
	map_ = new UndirectedGraph();
}

UndirectedGraphHandler::~UndirectedGraphHandler()
{}

CMapType UndirectedGraphHandler::type() const
{
	return CMapType::UNDIRECTED_GRAPH;
}

const UndirectedGraph* UndirectedGraphHandler::map() const
{
	return static_cast<const UndirectedGraph*>(map_);
}


UndirectedGraph* UndirectedGraphHandler::map()
{
	return static_cast<UndirectedGraph*>(map_);
}

void UndirectedGraphHandler::foreach_cell(cgogn::Orbit orb, const std::function<void (cgogn::Dart)>& func) const
{
	switch(orb)
	{
		case cgogn::Orbit::DART:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::DART> d) { func(d.dart); });
			break;
		case cgogn::Orbit::PHI1:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::PHI1> d) { func(d.dart); });
			break;
		case cgogn::Orbit::PHI2:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::PHI2> d) { func(d.dart); });
			break;
		case cgogn::Orbit::PHI21:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::PHI21> d) { func(d.dart); });
			break;
		default:
			cgogn_log_warning("UndirectedGraphHandler::foreach_cell") << "The orbit \"" << cgogn::orbit_name(orb) << "\" is not valid for this map type.";
			break;
	}
}

/*********************************************************
 * MANAGE DRAWING
 *********************************************************/


void UndirectedGraphHandler::draw(cgogn::rendering::DrawingType primitive)
{
	if (!render_.is_primitive_uptodate(primitive))
	{
		lock_topo_access();
		render_.init_primitives(*map(), primitive);
		unlock_topo_access();
	}
	render_.draw(primitive);
}

void UndirectedGraphHandler::init_primitives(cgogn::rendering::DrawingType primitive)
{
	render_.init_primitives(*map(), primitive);
}

/*********************************************************
 * MANAGE VBOs
 *********************************************************/

cgogn::rendering::VBO* UndirectedGraphHandler::create_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (!vbo)
	{
		if (!map_->has_attribute(UndirectedGraph::Vertex::ORBIT, name.toStdString()))
			return nullptr;

		const UndirectedGraph::VertexAttribute<VEC3F> va3f = map()->get_attribute<VEC3F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<VEC3D> va3d = map()->get_attribute<VEC3D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<float32> vaf32 = map()->get_attribute<float32, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<float64> vaf64 = map()->get_attribute<float64, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<VEC4F> va4f = map()->get_attribute<VEC4F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<VEC4D> va4d = map()->get_attribute<VEC4D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<VEC2F> va2f = map()->get_attribute<VEC2F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const UndirectedGraph::VertexAttribute<VEC2D> va2d = map()->get_attribute<VEC2D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

//		const UndirectedGraph::VertexAttribute<AVEC3D> ava3d = map()->get_attribute<AVEC3D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
//		if (ava3d.is_valid())
//		{
//			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3d, vbo);
//			emit(vbo_added(vbo));
//			return vbo;
//		}

//		const UndirectedGraph::VertexAttribute<AVEC3F> ava3f = map()->get_attribute<AVEC3F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
//		if (ava3f.is_valid())
//		{
//			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3f, vbo);
//			emit(vbo_added(vbo));
//			return vbo;
//		}
	}

	return vbo;
}

void UndirectedGraphHandler::update_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (vbo)
	{
		const UndirectedGraph::VertexAttribute<VEC3F> va3f = map()->get_attribute<VEC3F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<VEC3D> va3d = map()->get_attribute<VEC3D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<float32> vaf32 = map()->get_attribute<float32, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<float64> vaf64 = map()->get_attribute<float64, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<VEC4F> va4f = map()->get_attribute<VEC4F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<VEC4D> va4d = map()->get_attribute<VEC4D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<VEC2F> va2f = map()->get_attribute<VEC2F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			return;
		}

		const UndirectedGraph::VertexAttribute<VEC2D> va2d = map()->get_attribute<VEC2D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			return;
		}

//		const UndirectedGraph::VertexAttribute<AVEC3F> ava3f = map()->get_attribute<AVEC3F, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
//		if (ava3f.is_valid())
//		{
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3f, vbo);
//			return;
//		}

//		const UndirectedGraph::VertexAttribute<AVEC3D> ava3d = map()->get_attribute<AVEC3D, UndirectedGraph::Vertex::ORBIT>(name.toStdString());
//		if (ava3d.is_valid())
//		{
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3d, vbo);
//			return;
//		}
	}
}


/**********************************************************
 * MANAGE CELLS SETS                                      *
 *********************************************************/

CMapCellsSetGen* UndirectedGraphHandler::new_cell_set(cgogn::Orbit orbit, const QString& name)
{
	switch (orbit)
	{
		case UndirectedGraph::Vertex::ORBIT:
			return new UndirectedGraphCellsSet<UndirectedGraph::Vertex>(*this, name);
		case UndirectedGraph::Edge::ORBIT:
			return new UndirectedGraphCellsSet<UndirectedGraph::Edge>(*this, name);
		default:
			break;
	}
	return nullptr;
}


void UndirectedGraphHandler::compute_bb()
{
	this->bb_diagonal_size_ = .0f;
	this->bb_.reset();

	if (bb_vertex_attribute_ && bb_vertex_attribute_->is_valid())
		if(filtered())
		{
			auto* attr = dynamic_cast<cgogn::Attribute<VEC3, UndirectedGraph::Vertex::ORBIT>*>(bb_vertex_attribute_.get());
			cgogn::geometry::compute_AABB(*attr, *this->map(), *this->filter_, this->bb_);
		}
		else
			cgogn::geometry::compute_AABB(*bb_vertex_attribute_, this->bb_);
	if (this->bb_.is_initialized())
		this->bb_diagonal_size_ = cgogn::geometry::diagonal(this->bb_).norm();
}

std::unique_ptr<cgogn::Attribute_T<VEC3> > UndirectedGraphHandler::get_bb_vertex_attribute(const QString& attribute_name) const
{
	auto  attribute = map()->template get_attribute<VEC3, UndirectedGraph::Vertex::ORBIT>(attribute_name.toStdString());
	return std::unique_ptr< cgogn::Attribute<VEC3, UndirectedGraph::Vertex::ORBIT> >(new cgogn::Attribute<VEC3, UndirectedGraph::Vertex::ORBIT> (attribute));
}





} // namespace plugin_cmap_provider

} // namespace schnapps






