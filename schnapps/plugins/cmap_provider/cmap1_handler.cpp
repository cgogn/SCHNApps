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

#include <schnapps/plugins/cmap_provider/cmap1_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>
#include <schnapps/core/view.h>

#include <cgogn/rendering/shaders/vbo.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

CMap1Handler::CMap1Handler(const QString& name, PluginProvider* p) :
	CMapHandlerGen(name, p)
{
	map_ = new CMap1();
}

CMap1Handler::~CMap1Handler()
{}

void CMap1Handler::foreach_cell(cgogn::Orbit orb, const std::function<void (cgogn::Dart)>& func) const
{
	switch(orb)
	{
		case cgogn::Orbit::DART:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::DART> d) { func(d.dart); });
			break;
		case cgogn::Orbit::PHI1:
			map()->foreach_cell([&] (cgogn::Cell<cgogn::Orbit::PHI1> d) { func(d.dart); });
			break;
		default:
			cgogn_log_warning("CMap1Handler::foreach_cell") << "The orbit \"" << cgogn::orbit_name(orb) << "\" is not valid for this map type.";
			break;
	}
}

CMapType CMap1Handler::type() const
{
	return CMapType::CMAP1;
}

CMap1* CMap1Handler::map()
{
	return static_cast<CMap1*>(map_);
}

const CMap1* CMap1Handler::map() const
{
	return static_cast<const CMap1*>(map_);
}

/*********************************************************
 * MANAGE DRAWING
 *********************************************************/

void CMap1Handler::draw(cgogn::rendering::DrawingType primitive)
{
	if (!render_.is_primitive_uptodate(primitive))
	{
		lock_topo_access();
		render_.init_primitives(*map(), primitive);
		unlock_topo_access();
	}
	render_.draw(primitive);
}

void CMap1Handler::init_primitives(cgogn::rendering::DrawingType primitive)
{
	render_.init_primitives(*map(), primitive);
}

/*********************************************************
 * MANAGE VBOs
 *********************************************************/

cgogn::rendering::VBO* CMap1Handler::create_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (!vbo)
	{
		if (!map_->has_attribute(CMap1::Vertex::ORBIT, name.toStdString()))
			return nullptr;

		const CMap1::VertexAttribute<VEC3F> va3f = map()->get_attribute<VEC3F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<VEC3D> va3d = map()->get_attribute<VEC3D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<float32> vaf32 = map()->get_attribute<float32, CMap1::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<float64> vaf64 = map()->get_attribute<float64, CMap1::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<VEC4F> va4f = map()->get_attribute<VEC4F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<VEC4D> va4d = map()->get_attribute<VEC4D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<VEC2F> va2f = map()->get_attribute<VEC2F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap1::VertexAttribute<VEC2D> va2d = map()->get_attribute<VEC2D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

//		const CMap1::VertexAttribute<AVEC3D> ava3d = map()->get_attribute<AVEC3D, CMap1::Vertex::ORBIT>(name.toStdString());
//		if (ava3d.is_valid())
//		{
//			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3d, vbo);
//			emit(vbo_added(vbo));
//			return vbo;
//		}

//		const CMap1::VertexAttribute<AVEC3F> ava3f = map()->get_attribute<AVEC3F, CMap1::Vertex::ORBIT>(name.toStdString());
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

void CMap1Handler::update_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (vbo)
	{
		const CMap1::VertexAttribute<VEC3F> va3f = map()->get_attribute<VEC3F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			return;
		}

		const CMap1::VertexAttribute<VEC3D> va3d = map()->get_attribute<VEC3D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			return;
		}

		const CMap1::VertexAttribute<float32> vaf32 = map()->get_attribute<float32, CMap1::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			return;
		}

		const CMap1::VertexAttribute<float64> vaf64 = map()->get_attribute<float64, CMap1::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			return;
		}

		const CMap1::VertexAttribute<VEC4F> va4f = map()->get_attribute<VEC4F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			return;
		}

		const CMap1::VertexAttribute<VEC4D> va4d = map()->get_attribute<VEC4D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			return;
		}

		const CMap1::VertexAttribute<VEC2F> va2f = map()->get_attribute<VEC2F, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			return;
		}

		const CMap1::VertexAttribute<VEC2D> va2d = map()->get_attribute<VEC2D, CMap1::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			return;
		}

//		const CMap1::VertexAttribute<AVEC3F> ava3f = map()->get_attribute<AVEC3F, CMap1::Vertex::ORBIT>(name.toStdString());
//		if (ava3f.is_valid())
//		{
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3f, vbo);
//			return;
//		}

//		const CMap1::VertexAttribute<AVEC3D> ava3d = map()->get_attribute<AVEC3D, CMap1::Vertex::ORBIT>(name.toStdString());
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


CMapCellsSetGen*CMap1Handler::new_cell_set(cgogn::Orbit orbit, const QString& name)
{
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT:
			return new CMap1CellsSet<CMap1::Vertex>(*this, name);
		case CMap1::Face::ORBIT:
			return new CMap1CellsSet<CMap1::Face>(*this, name);
		default:
			break;
	}
	return nullptr;
}

void CMap1Handler::compute_bb()
{
	this->bb_diagonal_size_ = .0f;
	this->bb_.reset();

	if (bb_vertex_attribute_ && bb_vertex_attribute_->is_valid())
		if(filtered())
		{
			auto* attr = dynamic_cast<cgogn::Attribute<VEC3, CMap1::Vertex::ORBIT>*>(bb_vertex_attribute_.get());
			cgogn::geometry::compute_AABB(*attr, *this->map(), *this->filter_, this->bb_);
		}
		else
			cgogn::geometry::compute_AABB(*bb_vertex_attribute_, this->bb_);
	if (this->bb_.is_initialized())
		this->bb_diagonal_size_ = cgogn::geometry::diagonal(this->bb_).norm();
}

std::unique_ptr<cgogn::Attribute_T<VEC3> > CMap1Handler::get_bb_vertex_attribute(const QString& attribute_name) const
{
	auto  attribute = map()->template get_attribute<VEC3, CMap1::Vertex::ORBIT>(attribute_name.toStdString());
	return std::unique_ptr< cgogn::Attribute<VEC3, CMap1::Vertex::ORBIT> >(new cgogn::Attribute<VEC3, CMap1::Vertex::ORBIT> (attribute));
}


} // namespace plugin_cmap_provider

} // namespace schnapps