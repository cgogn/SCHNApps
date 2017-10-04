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

#include "surface_render.h"

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>
#endif

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_surface_render
{

MapParameters& Plugin_SurfaceRender::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_SurfaceRender::check_docktab_activation()
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

bool Plugin_SurfaceRender::enable()
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

	if (get_setting("Auto load color attribute").isValid())
		setting_auto_load_color_attribute_ = get_setting("Auto load color attribute").toString();
	else
		setting_auto_load_color_attribute_ = add_setting("Auto load color attribute", "color").toString();

	dock_tab_ = new SurfaceRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render");

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

#ifdef USE_TRANSPARENCY
	plugin_transparency_ = reinterpret_cast<PluginInteraction*>(schnapps_->enable_plugin("surface_render_transp"));
#endif

	return true;
}

void Plugin_SurfaceRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_SurfaceRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2)
	{
		view->makeCurrent();
		MapParameters& p = get_parameters(view, map);

		if (p.render_faces_ && !p.use_transparency_)
		{
			// apply polygon offset only when needed (edges over faces)
			if (p.render_edges_)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(0.5f, 1.0f);
			}
			if (p.position_vbo_)
			{
				if (p.color_vbo_)
				{
					switch (p.face_style_)
					{
						case MapParameters::FaceShadingStyle::FLAT:
							p.shader_flat_color_param_->bind(proj, mv);
							map->draw(cgogn::rendering::TRIANGLES);
							p.shader_flat_color_param_->release();
							break;
						case MapParameters::FaceShadingStyle::PHONG:
							if (p.normal_vbo_)
							{
								p.shader_phong_color_param_->bind(proj, mv);
								map->draw(cgogn::rendering::TRIANGLES);
								p.shader_phong_color_param_->release();
							}
							break;
					}
				}
				else
				{
					switch (p.face_style_)
					{
						case MapParameters::FaceShadingStyle::FLAT:
							p.shader_flat_param_->bind(proj, mv);
							map->draw(cgogn::rendering::TRIANGLES);
							p.shader_flat_param_->release();
							break;
						case MapParameters::FaceShadingStyle::PHONG:
							if (p.normal_vbo_)
							{
								p.shader_phong_param_->bind(proj, mv);
								map->draw(cgogn::rendering::TRIANGLES);
								p.shader_phong_param_->release();
							}
							break;
					}
				}
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
		}

		if (p.render_edges_)
		{
			if (p.position_vbo_)
			{
				p.shader_simple_color_param_->bind(proj, mv);
				map->draw(cgogn::rendering::LINES);
				p.shader_simple_color_param_->release();
			}
		}

		if (p.render_vertices_)
		{
			if (p.position_vbo_)
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				map->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}

		if (p.render_boundary_)
		{
			if (p.position_vbo_)
			{
				p.shader_simple_color_param_boundary_->bind(proj, mv);
				map->draw(cgogn::rendering::BOUNDARY);
				p.shader_simple_color_param_boundary_->release();
			}
		}
	}
}

void Plugin_SurfaceRender::view_linked(View* view)
{
#ifdef USE_TRANSPARENCY
	view->link_plugin(plugin_transparency_);
#endif

	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { add_linked_map(view, map); }
}

void Plugin_SurfaceRender::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { remove_linked_map(view, map); }
}

void Plugin_SurfaceRender::map_linked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	add_linked_map(view, map);
}

void Plugin_SurfaceRender::add_linked_map(View* view, MapHandlerGen *map)
{
	if (map->dimension() == 2)
	{
		set_position_vbo(view, map, map->get_vbo(setting_auto_load_position_attribute_), false);
		set_normal_vbo(view, map, map->get_vbo(setting_auto_load_normal_attribute_), false);
		set_color_vbo(view, map, map->get_vbo(setting_auto_load_color_attribute_), false);

#ifdef USE_TRANSPARENCY
		MapParameters& p = get_parameters(view, map);
		if (p.use_transparency_)
			add_transparency(view, map);
#endif

		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_SurfaceRender::map_unlinked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	remove_linked_map(view, map);
}

void Plugin_SurfaceRender::remove_linked_map(View* view, MapHandlerGen *map)
{
	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));

#ifdef USE_TRANSPARENCY
		MapParameters& p = get_parameters(view, map);
		if (p.use_transparency_)
			remove_transparency(view, map);
#endif

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_SurfaceRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					this->set_position_vbo(it.first, map, vbo, true);
				if (!p.normal_vbo_ && vbo_name == setting_auto_load_normal_attribute_)
					this->set_normal_vbo(it.first, map, vbo, true);
				if (!p.color_vbo_ && vbo_name == setting_auto_load_color_attribute_)
					this->set_color_vbo(it.first, map, vbo, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (vbo->vector_dimension() == 3)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.position_vbo_ == vbo)
					this->set_position_vbo(it.first, map, nullptr, true);
				if (p.normal_vbo_ == vbo)
					this->set_normal_vbo(it.first, map, nullptr, true);
				if (p.color_vbo_ == vbo)
					this->set_color_vbo(it.first, map, nullptr, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceRender::linked_map_bb_changed()
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());
	const uint32 nbe = map->nb_cells(Edge_Cell);

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(nbe)));
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_SurfaceRender::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && (parameter_set_.count(view) > 0))
	{
		auto& view_param_set = parameter_set_[view];
		for (auto& p : view_param_set)
		{
			MapHandlerGen* map = p.first;
			MapParameters& mp = p.second;
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				remove_transparency(view, map);
#endif
			mp.initialize_gl();
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				add_transparency(view, map);
#endif
		}
	}
}

#ifdef USE_TRANSPARENCY
void Plugin_SurfaceRender::add_transparency(View* view, MapHandlerGen* map)
{
	const MapParameters& p = get_parameters(view, map);
	if (p.face_style_ == MapParameters::FLAT)
		plugin_surface_render_transp::add_tr_flat(plugin_transparency_, view, map, p.get_transp_flat_param());
	else if (p.face_style_ == MapParameters::PHONG)
		plugin_surface_render_transp::add_tr_phong(plugin_transparency_, view, map, p.get_transp_phong_param());
}

void Plugin_SurfaceRender::remove_transparency(View* view, MapHandlerGen* map)
{
	const MapParameters& p = get_parameters(view, map);
	if (p.face_style_ == MapParameters::FLAT)
		plugin_surface_render_transp::remove_tr_flat(plugin_transparency_, view, map, p.get_transp_flat_param());
	else if (p.face_style_ == MapParameters::PHONG)
		plugin_surface_render_transp::remove_tr_phong(plugin_transparency_, view, map, p.get_transp_phong_param());
}

void Plugin_SurfaceRender::change_transparency(View* view, MapHandlerGen* map)
{
	const MapParameters& p = get_parameters(view, map);
	if (p.face_style_ == MapParameters::FLAT)
	{
		plugin_surface_render_transp::remove_tr_phong(plugin_transparency_, view, map, p.get_transp_phong_param());
		plugin_surface_render_transp::add_tr_flat(plugin_transparency_, view, map, p.get_transp_flat_param());
	}
	else if (p.face_style_ == MapParameters::PHONG)
	{
		plugin_surface_render_transp::remove_tr_flat(plugin_transparency_, view, map, p.get_transp_flat_param());
		plugin_surface_render_transp::add_tr_phong(plugin_transparency_, view, map, p.get_transp_phong_param());
	}
}
#endif

void Plugin_SurfaceRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->get_selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->get_selected_view()->link_plugin(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceRender::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_SurfaceRender::set_normal_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_normal_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_normal_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRender::set_color_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_color_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_color_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_vertices(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_vertices_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_vertices(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_edges(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_edges_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_edges(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_faces(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_faces_ = b;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
		{
			if (b)
				add_transparency(view, map);
			else
				remove_transparency(view, map);
		}
#endif
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_faces(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_backfaces(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_render_backfaces(b);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_backfaces(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_face_style(View* view, MapHandlerGen* map, MapParameters::FaceShadingStyle s, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.face_style_ = s;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
			change_transparency(view, map);
#endif
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_face_style(s);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_boundary(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_boundary_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_boundary(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_vertex_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_edge_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_edge_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_edge_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_front_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_front_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_front_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_back_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_back_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_back_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_scale_factor(sf);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_vertex_scale_factor(sf);
		view->update();
	}
}

void Plugin_SurfaceRender::set_transparency_enabled(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_transparency_enabled(b);
#ifdef USE_TRANSPARENCY
		if (p.render_faces_)
		{
			if (b)
				add_transparency(view, map);
			else
				remove_transparency(view, map);
		}
#endif
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_transparency_enabled(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_transparency_factor(View* view, MapHandlerGen* map, int32 tf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_transparency_factor(tf);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_transparency_factor(tf);
		view->update();
	}
}

} // namespace plugin_surface_render

} // namespace schnapps
