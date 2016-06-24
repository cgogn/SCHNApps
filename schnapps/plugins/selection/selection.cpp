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

#include <selection.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

MapParameters& Plugin_Selection::get_parameters(MapHandlerGen* map)
{
	MapParameters& p = parameter_set_[map];
	p.map_ = static_cast<MapHandler<CMap2>*>(map);
	return p;
}

bool Plugin_Selection::enable()
{
	dock_tab_ = new Selection_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Selection");

	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));

	return true;
}

void Plugin_Selection::disable()
{
}

void Plugin_Selection::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->is_selected_map())
	{
		view->makeCurrent();
		const MapParameters& p = get_parameters(map);
		if (p.get_position_attribute().is_valid())
		{

		}
	}
}

void Plugin_Selection::keyPress(View* view, QKeyEvent* e)
{

}

void Plugin_Selection::keyRelease(View* view, QKeyEvent* e)
{

}

void Plugin_Selection::mousePress(View* view, QMouseEvent* e)
{

}

void Plugin_Selection::mouseMove(View* view, QMouseEvent* e)
{

}

void Plugin_Selection::wheelEvent(View* view, QWheelEvent* e)
{

}

void Plugin_Selection::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (old)
	{
		disconnect(old, SIGNAL(cells_set_added(CellType, const QString&)), dock_tab_, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		disconnect(old, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), dock_tab_, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(old, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(old, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		disconnect(old, SIGNAL(bb_changed()), this, SLOT(selected_map_bb_changed()));
	}
	if (cur)
	{
		connect(cur, SIGNAL(cells_set_added(CellType, const QString&)), dock_tab_, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		connect(cur, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), dock_tab_, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(cur, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		connect(cur, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		connect(cur, SIGNAL(bb_changed()), this, SLOT(selected_map_bb_changed()));

		const MapParameters& p = get_parameters(cur);
		dock_tab_->update_map_parameters(cur, p);
	}
}

void Plugin_Selection::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
	MapParameters& p = get_parameters(map);
	if (p.get_position_attribute_name() == name)
		p.set_position_attribute("");
	if (p.get_normal_attribute_name() == name)
		p.set_normal_attribute("");

	dock_tab_->selected_map_attribute_removed(orbit, name);
}

void Plugin_Selection::selected_map_connectiviy_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
	MapParameters& p = get_parameters(map);
//	if (p.get_position_attribute().is_valid())
//		update_selected_cells_rendering();
}

void Plugin_Selection::selected_map_bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
	MapParameters& p = get_parameters(map);
	p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
