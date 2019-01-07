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

#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>

#include <schnapps/core/view.h>

#include <cgogn/rendering/shaders/vbo.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

CMap2Handler::CMap2Handler(const QString& name, PluginProvider* p) :
	Object(name, p)
{
	map_ = new CMap2();
}

CMap2Handler::~CMap2Handler()
{
	delete map_;
}

/*********************************************************
 * MANAGE DRAWING
 *********************************************************/

void CMap2Handler::draw(cgogn::rendering::DrawingType primitive)
{
	if (!render_.is_primitive_uptodate(primitive))
	{
		lock_topo_access();
		render_.init_primitives(*map_, primitive);
		unlock_topo_access();
	}
	render_.draw(primitive);
}

void CMap2Handler::init_primitives(cgogn::rendering::DrawingType primitive)
{
	render_.init_primitives(*map_, primitive);
}

/*********************************************************
 * MANAGE VBOs
 *********************************************************/

cgogn::rendering::VBO* CMap2Handler::create_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (!vbo)
	{
		if (!map_->has_attribute(CMap2::Vertex::ORBIT, name.toStdString()))
			return nullptr;

		const CMap2::VertexAttribute<VEC3F> va3f = map_->get_attribute<VEC3F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<VEC3D> va3d = map_->get_attribute<VEC3D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<float32> vaf32 = map_->get_attribute<float32, CMap2::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<float64> vaf64 = map_->get_attribute<float64, CMap2::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<VEC4F> va4f = map_->get_attribute<VEC4F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<VEC4D> va4d = map_->get_attribute<VEC4D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<VEC2F> va2f = map_->get_attribute<VEC2F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

		const CMap2::VertexAttribute<VEC2D> va2d = map_->get_attribute<VEC2D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			emit(vbo_added(vbo));
			return vbo;
		}

//		const CMap2::VertexAttribute<AVEC3D> ava3d = map_->get_attribute<AVEC3D, CMap2::Vertex::ORBIT>(name.toStdString());
//		if (ava3d.is_valid())
//		{
//			vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3d, vbo);
//			emit(vbo_added(vbo));
//			return vbo;
//		}

//		const CMap2::VertexAttribute<AVEC3F> ava3f = map_->get_attribute<AVEC3F, CMap2::Vertex::ORBIT>(name.toStdString());
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

void CMap2Handler::update_vbo(const QString& name)
{
	cgogn::rendering::VBO* vbo = this->vbo(name);
	if (vbo)
	{
		const CMap2::VertexAttribute<VEC3F> va3f = map_->get_attribute<VEC3F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va3f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3f, vbo);
			return;
		}

		const CMap2::VertexAttribute<VEC3D> va3d = map_->get_attribute<VEC3D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va3d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va3d, vbo);
			return;
		}

		const CMap2::VertexAttribute<float32> vaf32 = map_->get_attribute<float32, CMap2::Vertex::ORBIT>(name.toStdString());
		if (vaf32.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf32, vbo);
			return;
		}

		const CMap2::VertexAttribute<float64> vaf64 = map_->get_attribute<float64, CMap2::Vertex::ORBIT>(name.toStdString());
		if (vaf64.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(vaf64, vbo);
			return;
		}

		const CMap2::VertexAttribute<VEC4F> va4f = map_->get_attribute<VEC4F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va4f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4f, vbo);
			return;
		}

		const CMap2::VertexAttribute<VEC4D> va4d = map_->get_attribute<VEC4D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va4d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va4d, vbo);
			return;
		}

		const CMap2::VertexAttribute<VEC2F> va2f = map_->get_attribute<VEC2F, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va2f.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2f, vbo);
			return;
		}

		const CMap2::VertexAttribute<VEC2D> va2d = map_->get_attribute<VEC2D, CMap2::Vertex::ORBIT>(name.toStdString());
		if (va2d.is_valid())
		{
			vbo = vbos_.at(name).get();
			cgogn::rendering::update_vbo(va2d, vbo);
			return;
		}

//		const CMap2::VertexAttribute<AVEC3F> ava3f = map_->get_attribute<AVEC3F, CMap2::Vertex::ORBIT>(name.toStdString());
//		if (ava3f.is_valid())
//		{
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3f, vbo);
//			return;
//		}

//		const CMap2::VertexAttribute<AVEC3D> ava3d = map_->get_attribute<AVEC3D, CMap2::Vertex::ORBIT>(name.toStdString());
//		if (ava3d.is_valid())
//		{
//			vbo = vbos_.at(name).get();
//			cgogn::rendering::update_vbo(ava3d, vbo);
//			return;
//		}
	}
}

cgogn::rendering::VBO* CMap2Handler::vbo(const QString& name) const
{
	if (vbos_.count(name) > 0ul)
		return vbos_.at(name).get();
	else
		return nullptr;
}

void CMap2Handler::delete_vbo(const QString &name)
{
	if (vbos_.count(name) > 0ul)
	{
		auto vbo = std::move(vbos_.at(name));
		vbos_.erase(name);
		emit(vbo_removed(vbo.get()));
	}
}

/**********************************************************
 * MANAGE CELLS SETS                                      *
 *********************************************************/

CMapCellsSetGen* CMap2Handler::add_cells_set(cgogn::Orbit orbit, const QString& name)
{
	if (cells_sets_[orbit].count(name) > 0ul)
		return nullptr;

	CMapCellsSetGen* cells_set = nullptr;

	switch (orbit)
	{
		case CMap2::CDart::ORBIT:
			cells_set = new CMap2CellsSet<CMap2::CDart>(*this, name);
			break;
		case CMap2::Vertex::ORBIT:
			cells_set = new CMap2CellsSet<CMap2::Vertex>(*this, name);
			break;
		case CMap2::Edge::ORBIT:
			cells_set = new CMap2CellsSet<CMap2::Edge>(*this, name);
			break;
		case CMap2::Face::ORBIT:
			cells_set = new CMap2CellsSet<CMap2::Face>(*this, name);
			break;
		case CMap2::Volume::ORBIT:
			cells_set = new CMap2CellsSet<CMap2::Volume>(*this, name);
			break;
		default:
			break;
	}

	if (cells_set)
	{
		cells_sets_[orbit].insert(std::make_pair(name, cells_set));
		emit(cells_set_added(orbit, name));
		connect(this, SIGNAL(connectivity_changed()), cells_set, SLOT(rebuild()));
	}

	return cells_set;
}

void CMap2Handler::remove_cells_set(cgogn::Orbit orbit, const QString& name)
{
	const auto cells_set_it = cells_sets_[orbit].find(name);
	if (cells_set_it == cells_sets_[orbit].end())
		return;

	disconnect(this, SIGNAL(connectivity_changed()), cells_set_it->second, SLOT(rebuild()));

	emit(cells_set_removed(orbit, name));
	delete cells_set_it->second;
	cells_sets_[orbit].erase(cells_set_it);
}

/*********************************************************
 * MANAGE ATTRIBUTES & CONNECTIVITY
 *********************************************************/

void CMap2Handler::notify_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	emit(attribute_added(orbit, attribute_name));
}

void CMap2Handler::notify_attribute_change(cgogn::Orbit orbit, const QString& attribute_name)
{
	update_vbo(attribute_name);
	check_bb_vertex_attribute(orbit, attribute_name);

	emit(attribute_changed(orbit, attribute_name));

	for (View* view : views_)
		view->update();
}

void CMap2Handler::notify_connectivity_change()
{
	render_.set_primitive_dirty(cgogn::rendering::POINTS);
	render_.set_primitive_dirty(cgogn::rendering::LINES);
	render_.set_primitive_dirty(cgogn::rendering::TRIANGLES);
	render_.set_primitive_dirty(cgogn::rendering::BOUNDARY);

	emit(connectivity_changed());

	for (View* view : views_)
		view->update();
}

} // namespace plugin_cmap_provider

} // namespace schnapps
