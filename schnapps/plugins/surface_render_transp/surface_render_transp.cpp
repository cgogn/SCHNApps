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

#include <surface_render_transp.h>

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_surface_render_transp
{

Plugin_SurfaceRenderTransp::~Plugin_SurfaceRenderTransp()
{}

MapParameters& Plugin_SurfaceRenderTransp::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_SurfaceRenderTransp::enable()
{
	if (get_setting("Auto enable on selected view").isValid())
		setting_auto_enable_on_selected_view_ = get_setting("Auto enable on selected view").toBool();
	else
		setting_auto_enable_on_selected_view_ = add_setting("Auto enable on selected view", true).toBool();

	if (get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	if (get_setting("Auto load normal attribute").isValid())
		setting_auto_load_normal_attribute_ = get_setting("Auto load normal attribute").toString();
	else
		setting_auto_load_normal_attribute_ = add_setting("Auto load normal attribute", "normal").toString();

	dock_tab_ = new SurfaceRenderTransp_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Transp");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

	update_dock_tab();

	return true;
}

void Plugin_SurfaceRenderTransp::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_SurfaceRenderTransp::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2)
	{
		//save map
		const MapParameters& p = get_parameters(view, map);
	}
}

void Plugin_SurfaceRenderTransp::draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	const std::list<MapHandlerGen*>& maps = view->get_linked_maps();

	cgogn::rendering::SurfaceTransparencyDrawer trdr = transp_drawer_set_[view];

	trdr->draw_flat(proj,mv, [&]
	{
		for (const auto& m: maps)
		{
			m->draw(cgogn::rendering::TRIANGLES);
		}
	});

}

void Plugin_SurfaceRenderTransp::resizeGL(View* view, int width, int height)
{
	transp_drawer_->resize(view->devicePixelRatio()*width,view->devicePixelRatio()*height,this);
//	QOGLViewer::resizeGL(width,height);
}
void Plugin_SurfaceRenderTransp::view_linked(View* view)
{
	update_dock_tab();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_linked(map); }
}

void Plugin_SurfaceRenderTransp::view_unlinked(View* view)
{
	update_dock_tab();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_unlinked(map); }
}

void Plugin_SurfaceRenderTransp::map_linked(MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		View* view = schnapps_->get_selected_view();
		if (view)
		{
			set_position_vbo(view->get_name(), map->get_name(), setting_auto_load_position_attribute_);
			set_normal_vbo(view->get_name(), map->get_name(), setting_auto_load_normal_attribute_);
		}

		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	}
}

void Plugin_SurfaceRenderTransp::map_unlinked(MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	}
}

void Plugin_SurfaceRenderTransp::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (map && map->is_selected_map())
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		if (vbo->vector_dimension() == 3)
		{
			View* view = schnapps_->get_selected_view();
			dock_tab_->add_position_vbo(vbo_name);
			dock_tab_->add_normal_vbo(vbo_name);
			if (view)
			{
				if (!get_parameters(view, map).get_position_vbo() && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(view->get_name(), map->get_name(), vbo_name);
				if (!get_parameters(view, map).get_normal_vbo() && vbo_name == setting_auto_load_normal_attribute_)
					set_normal_vbo(view->get_name(), map->get_name(), vbo_name);
			}
		}
	}
}

void Plugin_SurfaceRenderTransp::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (map && map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_normal_vbo(QString::fromStdString(vbo->name()));
		}
	}

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			if (p.get_position_vbo() == vbo)
				p.set_position_vbo(nullptr);
			if (p.get_normal_vbo() == vbo)
				p.set_normal_vbo(nullptr);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}



void Plugin_SurfaceRenderTransp::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && (this->parameter_set_.count(view) > 0))
	{
		auto& view_param_set = parameter_set_[view];
		for (auto & p : view_param_set)
			p.second.initialize_gl();
		transp_drawer_set_[view]= new cgogn::rendering::SurfaceTransparencyDrawer();
	}
}

void Plugin_SurfaceRenderTransp::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->get_selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->get_selected_view()->link_plugin(this);
}

void Plugin_SurfaceRenderTransp::update_dock_tab()
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


void Plugin_SurfaceRenderTransp::set_face_style(View* view, MapHandlerGen* map, MapParameters::FaceShadingStyle s)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.face_style_ = s;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRenderTransp::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_vbo(vbo);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRenderTransp::set_normal_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_normal_vbo(vbo);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

} // namespace plugin_surface_render_transp

} // namespace schnapps
