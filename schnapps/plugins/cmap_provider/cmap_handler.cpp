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

#include <schnapps/plugins/cmap_provider/cmap_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>

#include <schnapps/core/view.h>


namespace schnapps
{

namespace plugin_cmap_provider
{

CMapHandlerGen::CMapHandlerGen(const QString& name, PluginProvider* p) :
	Object(name, p),
	map_(nullptr),
	topo_access_(),
	render_(),
	vbos_(),
	cells_sets_(),
	bb_vertex_attribute_(nullptr)
{}

CMapHandlerGen::~CMapHandlerGen()
{
	bb_vertex_attribute_.reset();
	delete map_;
}

MapBaseData*CMapHandlerGen::map()
{
	return map_;
}

const MapBaseData*CMapHandlerGen::map() const
{
	return map_;
}

void CMapHandlerGen::view_linked(View*)
{}

void CMapHandlerGen::view_unlinked(View*)
{}

cgogn::rendering::VBO*CMapHandlerGen::vbo(const QString& name) const
{
	if (vbos_.count(name) > 0ul)
		return vbos_.at(name).get();
	else
		return nullptr;
}

void CMapHandlerGen::delete_vbo(const QString& name)
{
	if (vbos_.count(name) > 0ul)
	{
		auto vbo = std::move(vbos_.at(name));
		vbos_.erase(name);
		emit(vbo_removed(vbo.get()));
	}
}

void CMapHandlerGen::foreach_vbo(const std::function<void (cgogn::rendering::VBO*)>& f) const
{
	for (const auto& vbo_it : vbos_)
		f(vbo_it.second.get());
}

CMapCellsSetGen* CMapHandlerGen::add_cells_set(cgogn::Orbit orbit, const QString& name)
{
	if (this->cells_sets_[orbit].count(name) > 0ul)
		return nullptr;

	CMapCellsSetGen* cells_set = new_cell_set(orbit, name);

	if (cells_set)
	{
		cells_sets_[orbit].insert(std::make_pair(name, cells_set));
		emit(cells_set_added(orbit, name));
		connect(this, SIGNAL(connectivity_changed()), cells_set, SLOT(rebuild()));
	}

	return cells_set;
}


void CMapHandlerGen::remove_cells_set(cgogn::Orbit orbit, const QString& name)
{
	const auto cells_set_it = cells_sets_[orbit].find(name);
	if (cells_set_it == cells_sets_[orbit].end())
		return;

	disconnect(this, SIGNAL(connectivity_changed()), cells_set_it->second, SLOT(rebuild()));

	emit(cells_set_removed(orbit, name));
	delete cells_set_it->second;
	cells_sets_[orbit].erase(cells_set_it);
}

CMapCellsSetGen* CMapHandlerGen::cells_set(cgogn::Orbit orbit, const QString& name)
{
	if (cells_sets_[orbit].count(name) > 0ul)
		return cells_sets_[orbit].at(name);
	else
		return nullptr;
}


void CMapHandlerGen::foreach_cells_set(cgogn::Orbit orbit, const std::function<void (CMapCellsSetGen*)>& f) const
{
	for (const auto& cells_set_it : cells_sets_[orbit])
		f(cells_set_it.second);
}

void CMapHandlerGen::notify_cells_set_mutually_exclusive_change(cgogn::Orbit orb, const QString& name) const
{
	emit(cells_set_mutually_exclusive_changed(orb, name));
}


/*********************************************************
 * MANAGE BOUNDING BOX
 *********************************************************/

QString CMapHandlerGen::bb_vertex_attribute_name() const
{
	if (bb_vertex_attribute_ && bb_vertex_attribute_->is_valid())
		return QString::fromStdString(bb_vertex_attribute_->name());
	else
		return QString();
}

void CMapHandlerGen::set_bb_vertex_attribute(const QString& attribute_name)
{
	bb_vertex_attribute_ = this->get_bb_vertex_attribute(attribute_name);
	if (bb_vertex_attribute_&& bb_vertex_attribute_->is_valid())
	{
		compute_bb();
		this->update_bb_drawer();
		emit(bb_vertex_attribute_changed(attribute_name));
		emit(bb_changed());
	}
}

void CMapHandlerGen::check_bb_vertex_attribute(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (bb_vertex_attribute_ && bb_vertex_attribute_->is_valid())
	{
		if (orbit == bb_vertex_attribute_->orbit() && attribute_name == QString::fromStdString(bb_vertex_attribute_->name()))
		{
			compute_bb();
			this->update_bb_drawer();
			emit(bb_changed());
		}
	}
}

void CMapHandlerGen::compute_bb()
{
	this->bb_diagonal_size_ = .0f;
	this->bb_.reset();

	if (bb_vertex_attribute_ && bb_vertex_attribute_->is_valid())
		cgogn::geometry::compute_AABB(*bb_vertex_attribute_, this->bb_);
	if (this->bb_.is_initialized())
		this->bb_diagonal_size_ = cgogn::geometry::diagonal(this->bb_).norm();
}

bool CMapHandlerGen::remove_attribute(cgogn::Orbit orbit, const QString& att_name)
{
	const bool res = map()->remove_attribute(orbit, att_name.toStdString());
	if (res)
		emit(attribute_removed(orbit, att_name));
	return res;
}

void CMapHandlerGen::lock_topo_access()
{
	topo_access_.lock();
}

void CMapHandlerGen::unlock_topo_access()
{
	topo_access_.unlock();
}

/*********************************************************
 * MANAGE ATTRIBUTES & CONNECTIVITY
 *********************************************************/

void CMapHandlerGen::notify_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	emit(attribute_added(orbit, attribute_name));
}

void CMapHandlerGen::notify_attribute_change(cgogn::Orbit orbit, const QString& attribute_name)
{
	update_vbo(attribute_name);
	check_bb_vertex_attribute(orbit, attribute_name);

	emit(attribute_changed(orbit, attribute_name));

	for (View* view : views_)
		view->update();
}

void CMapHandlerGen::notify_connectivity_change()
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
