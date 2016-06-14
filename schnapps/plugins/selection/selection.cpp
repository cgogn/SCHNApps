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

void Plugin_Selection::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	const MapParameters& p = get_parameters(cur);
	dock_tab_->update_map_parameters(cur, p);
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
