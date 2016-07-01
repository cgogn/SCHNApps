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

#include <surface_render_vector.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

MapParameters& Plugin_SurfaceRenderVector::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	MapParameters& p = parameter_set_[view][map];
	p.map_ = static_cast<MapHandler<CMap2>*>(map);
	return p;
}

bool Plugin_SurfaceRenderVector::enable()
{
	dock_tab_ = new SurfaceRenderVector_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Vector");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));

	update_dock_tab();

	return true;
}

void Plugin_SurfaceRenderVector::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

void Plugin_SurfaceRenderVector::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2)
	{
		view->makeCurrent();
		const MapParameters& p = get_parameters(view, map);

		if (p.get_position_vbo())
		{
			for (auto& param : p.get_shader_params())
			{
				param->bind(proj, mv);
				map->draw(cgogn::rendering::POINTS);
				param->release();
			}
		}
	}
}

void Plugin_SurfaceRenderVector::view_linked(View* view)
{
	update_dock_tab();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_linked(map); }
}

void Plugin_SurfaceRenderVector::view_unlinked(View* view)
{
	update_dock_tab();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_unlinked(map); }
}

void Plugin_SurfaceRenderVector::map_linked(MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
	}
}

void Plugin_SurfaceRenderVector::map_unlinked(MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
	}
}

void Plugin_SurfaceRenderVector::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->add_vector_vbo(QString::fromStdString(vbo->name()));
		}
	}
}

void Plugin_SurfaceRenderVector::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_vector_vbo(QString::fromStdString(vbo->name()));
		}
	}

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& map_param = view_param_set[map];
			if (map_param.get_position_vbo() == vbo)
				map_param.set_position_vbo(nullptr);
			if (map_param.get_vector_vbo_index(vbo) >= 0ul)
				map_param.remove_vector_vbo(vbo);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_SurfaceRenderVector::linked_map_bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0)
		{
			MapParameters& p = view_param_set[map];
			for (uint32 i = 0, size = p.vector_scale_factor_list_.size(); i < size; ++i)
				p.set_vector_scale_factor(i, p.vector_scale_factor_list_[i]);
		}
	}
}

void Plugin_SurfaceRenderVector::update_dock_tab()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();
	if (view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		schnapps_->enable_plugin_tab_widgets(this);
		const MapParameters& p = get_parameters(view, map);
		dock_tab_->update_map_parameters(map, p);
	}
	else
		schnapps_->disable_plugin_tab_widgets(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceRenderVector::set_position_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
{
	View* view = schnapps_->get_view(view_name);
	MapHandlerGen* map = schnapps_->get_map(map_name);
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_vbo(map->get_vbo(vbo_name));
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRenderVector::add_vector_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
{
	View* view = schnapps_->get_view(view_name);
	MapHandlerGen* map = schnapps_->get_map(map_name);
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.add_vector_vbo(map->get_vbo(vbo_name));
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRenderVector::remove_vector_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
{
	View* view = schnapps_->get_view(view_name);
	MapHandlerGen* map = schnapps_->get_map(map_name);
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.remove_vector_vbo(map->get_vbo(vbo_name));
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRenderVector::set_vector_scale_factor(const QString& view_name, const QString& map_name, const QString& vector_vbo_name, float32 sf)
{
	View* view = schnapps_->get_view(view_name);
	MapHandlerGen* map = schnapps_->get_map(map_name);
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		int32 index = p.get_vector_vbo_index(map->get_vbo(vector_vbo_name));
		if (index >= 0)
		{
			p.set_vector_scale_factor(index, sf);
			if (view->is_selected_view() && map->is_selected_map())
				dock_tab_->update_map_parameters(map, p);
			view->update();
		}
	}
}

void Plugin_SurfaceRenderVector::set_vector_color(const QString& view_name, const QString& map_name, const QString& vector_vbo_name, const QColor& color)
{
	View* view = schnapps_->get_view(view_name);
	MapHandlerGen* map = schnapps_->get_map(map_name);
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		int32 index = p.get_vector_vbo_index(map->get_vbo(vector_vbo_name));
		if (index >= 0)
		{
			p.set_vector_color(index, color);
			if (view->is_selected_view() && map->is_selected_map())
				dock_tab_->update_map_parameters(map, p);
			view->update();
		}
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
