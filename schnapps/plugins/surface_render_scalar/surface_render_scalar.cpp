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

#include <schnapps/plugins/surface_render_scalar/surface_render_scalar.h>
#include <schnapps/plugins/surface_render_scalar/surface_render_scalar_dock_tab.h>

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/geometry/algos/area.h>
#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_surface_render_scalar
{

Plugin_SurfaceRenderScalar::Plugin_SurfaceRenderScalar()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceRenderScalar::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_SurfaceRenderScalar::get_parameters(View* view, MapHandlerGen* map)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(map, "Try to access parameters for null map");
	cgogn_message_assert(map->dimension() == 2, "Try to access parameters for map with dimension other than 2");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.map_ = static_cast<CMap2Handler*>(map);
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_SurfaceRenderScalar::check_docktab_activation()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		schnapps_->enable_plugin_tab_widgets(this);
		return true;
	}
	else
	{
		schnapps_->disable_plugin_tab_widgets(this);
		return false;
	}
}

bool Plugin_SurfaceRenderScalar::enable()
{
	if (get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new SurfaceRenderScalar_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Scalar");

	return true;
}

void Plugin_SurfaceRenderScalar::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

void Plugin_SurfaceRenderScalar::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2)
	{
		view->makeCurrent();
		const MapParameters& p = get_parameters(view, map);

		if (p.position_vbo_ && p.scalar_vbo_)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0f, 1.0f);
			p.shader_scalar_per_vertex_param_->bind(proj, mv);
			map->draw(cgogn::rendering::TRIANGLES);
			p.shader_scalar_per_vertex_param_->release();
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
}

void Plugin_SurfaceRenderScalar::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { add_linked_map(view, map); }
}

void Plugin_SurfaceRenderScalar::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { remove_linked_map(view, map); }
}

void Plugin_SurfaceRenderScalar::map_linked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	add_linked_map(view, map);
}

void Plugin_SurfaceRenderScalar::add_linked_map(View* view, MapHandlerGen *map)
{
	if (map->dimension() == 2)
	{
		set_position_vbo(view, map, map->get_vbo(setting_auto_load_position_attribute_), true);

		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, QString)), Qt::UniqueConnection);

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_SurfaceRenderScalar::map_unlinked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	remove_linked_map(view, map);
}

void Plugin_SurfaceRenderScalar::remove_linked_map(View* view, MapHandlerGen *map)
{
	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
		disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, QString)));

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_SurfaceRenderScalar::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				const MapParameters& p = view_param_set[map];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(it.first, map, vbo, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderScalar::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3 || vbo->vector_dimension() == 1)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, map, nullptr, true);
				if (p.scalar_vbo_ == vbo)
					set_scalar_vbo(it.first, map, nullptr, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderScalar::linked_map_bb_changed()
{

}

void Plugin_SurfaceRenderScalar::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.scalar_vbo_ && p.auto_update_min_max_ && QString::fromStdString(p.scalar_vbo_->name()) == attribute_name)
					update_min_max(it.first, map, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderScalar::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && (this->parameter_set_.count(view) > 0))
	{
		auto& view_param_set = parameter_set_[view];
		for (auto& p : view_param_set)
			p.second.initialize_gl();
	}
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceRenderScalar::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_position_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_scalar_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_scalar_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_scalar_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_color_map(View* view, MapHandlerGen* map, cgogn::rendering::ShaderScalarPerVertex::ColorMap cm, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_color_map(cm);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_color_map(cm);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_auto_update_min_max(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_auto_update_min_max(b);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_auto_update_min_max(b);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_scalar_min(View* view, MapHandlerGen* map, double d, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_scalar_min(d);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_scalar_min(d);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_scalar_max(View* view, MapHandlerGen* map, double d, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_scalar_max(d);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_scalar_max(d);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_expansion(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_expansion(i);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_expansion(i);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_show_iso_lines(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_show_iso_lines(b);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_show_iso_lines(b);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_nb_iso_levels(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_nb_iso_levels(i);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_nb_iso_levels(i);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::update_min_max(View* view, MapHandlerGen* map, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.update_min_max();
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
		{
			dock_tab_->set_scalar_min(p.get_scalar_min());
			dock_tab_->set_scalar_max(p.get_scalar_max());
		}
		view->update();
	}
}

} // namespace plugin_surface_render_scalar

} // namespace schnapps
