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
#ifdef USE_TRANSP
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>
#endif

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_surface_render
{

Plugin_SurfaceRender::~Plugin_SurfaceRender()
{}

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

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

	update_dock_tab();

#ifdef USE_TRANSP
	plug_transp_ = reinterpret_cast<PluginInteraction*>(schnapps_->enable_plugin("surface_render_transp"));
#endif
	return true;
}

void Plugin_SurfaceRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_SurfaceRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2)
	{
		view->makeCurrent();
		MapParameters& p = get_parameters(view, map);

		if ((p.render_faces_) &&(!p.use_transparency_))
		{
			// apply polygon offset only when needed (edges over faces
			if (p.render_edges_)
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(0.5f, 1.0f);
			}
			if (p.get_position_vbo())
			{
				if (p.get_color_vbo())
				{
					switch (p.face_style_)
					{
						case MapParameters::FaceShadingStyle::FLAT:
							p.shader_flat_color_param_->bind(proj, mv);
							map->draw(cgogn::rendering::TRIANGLES);
							p.shader_flat_color_param_->release();
							break;
						case MapParameters::FaceShadingStyle::PHONG:
							if (p.get_normal_vbo())
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
							if (p.get_normal_vbo())
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
			if (p.get_position_vbo())
			{
				p.shader_simple_color_param_->bind(proj, mv);
				map->draw(cgogn::rendering::LINES);
				p.shader_simple_color_param_->release();
			}
		}

		if (p.render_vertices_)
		{
			if (p.get_position_vbo())
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				map->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}

		if (p.render_boundary_)
		{
			if (p.get_position_vbo())
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
#ifdef USE_TRANSP
	view->link_plugin(plug_transp_);
#endif
	update_dock_tab();

	connection_map_linked_[view] = connect(view, &View::map_linked, [=] (MapHandlerGen* m) { this->map_linked(view,m);});
	connection_map_unlinked_[view] = connect(view, &View::map_unlinked, [=] (MapHandlerGen* m) { map_unlinked(view,m);});
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_linked(view,map); }
}

void Plugin_SurfaceRender::view_unlinked(View* view)
{
	update_dock_tab();

	disconnect(connection_map_linked_[view]);
	connection_map_linked_.erase(view);
	disconnect(connection_map_unlinked_[view]);
	connection_map_unlinked_.erase(view);
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_unlinked(view,map); }

	parameter_set_.erase(view);
}

void Plugin_SurfaceRender::map_linked(View* view, MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		set_position_vbo(view->get_name(), map->get_name(), setting_auto_load_position_attribute_);
		set_normal_vbo(view->get_name(), map->get_name(), setting_auto_load_normal_attribute_);
		set_color_vbo(view->get_name(), map->get_name(), setting_auto_load_color_attribute_);

		MapParameters& p = get_parameters(view, map);
#ifdef USE_TRANSP
		if (p.use_transparency_)
			add_transparency(view, map, p);
#endif
		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
	}
}

void Plugin_SurfaceRender::map_unlinked(View* view, MapHandlerGen *map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));

		MapParameters& p = get_parameters(view, map);
#ifdef USE_TRANSP
		if (p.use_transparency_)
			remove_transparency(view, map, p);
#endif
	}
}

void Plugin_SurfaceRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
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
			dock_tab_->add_color_vbo(vbo_name);
			if (view)
			{
				if (!get_parameters(view, map).get_position_vbo() && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(view->get_name(), map->get_name(), vbo_name);
				if (!get_parameters(view, map).get_normal_vbo() && vbo_name == setting_auto_load_normal_attribute_)
					set_normal_vbo(view->get_name(), map->get_name(), vbo_name);
				if (!get_parameters(view, map).get_color_vbo() && vbo_name == setting_auto_load_color_attribute_)
					set_color_vbo(view->get_name(), map->get_name(), vbo_name);
			}
		}
	}
}

void Plugin_SurfaceRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (map && map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_normal_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_color_vbo(QString::fromStdString(vbo->name()));
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
			if (p.get_color_vbo() == vbo)
				p.set_color_vbo(nullptr);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
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
}

void Plugin_SurfaceRender::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && (this->parameter_set_.count(view) > 0))
	{
		auto& view_param_set = parameter_set_[view];
		for (auto & p : view_param_set)
		{
			MapParameters& mp = p.second;
			MapHandlerGen* map = p.first;
#ifdef USE_TRANSP
			if (mp.use_transparency_)
				remove_transparency(view, map, mp);
#endif
			mp.initialize_gl();
#ifdef USE_TRANSP
			if (mp.use_transparency_)
				add_transparency(view, map, mp);
#endif
		}
	}
	update_dock_tab();
}

#ifdef USE_TRANSP
void Plugin_SurfaceRender::add_transparency(View* view,MapHandlerGen* map, MapParameters& mp)
{
	if (mp.face_style_ == MapParameters::FLAT)
		plugin_surface_render_transp::add_tr_flat(plug_transp_, view, map, mp.get_transp_flat_param());
	if (mp.face_style_ == MapParameters::PHONG)
		plugin_surface_render_transp::add_tr_phong(plug_transp_, view, map, mp.get_transp_phong_param());
}

void Plugin_SurfaceRender::remove_transparency(View* view, MapHandlerGen* map, MapParameters& mp)
{
	if (mp.face_style_ == MapParameters::FLAT)
		plugin_surface_render_transp::remove_tr_flat(plug_transp_, view, map, mp.get_transp_flat_param());
	if (mp.face_style_ == MapParameters::PHONG)
		plugin_surface_render_transp::remove_tr_phong(plug_transp_, view, map, mp.get_transp_phong_param());
}

void Plugin_SurfaceRender::change_transparency(View* view, MapHandlerGen* map, MapParameters& mp)
{
	if (mp.face_style_ == MapParameters::FLAT)
	{
		plugin_surface_render_transp::remove_tr_phong(plug_transp_, view, map, mp.get_transp_phong_param());
		plugin_surface_render_transp::add_tr_flat(plug_transp_, view, map, mp.get_transp_flat_param());
	}
	else if (mp.face_style_ == MapParameters::PHONG)
	{
		plugin_surface_render_transp::remove_tr_flat(plug_transp_, view, map, mp.get_transp_flat_param());
		plugin_surface_render_transp::add_tr_phong(plug_transp_, view, map, mp.get_transp_phong_param());
	}
}
#endif


void Plugin_SurfaceRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->get_selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->get_selected_view()->link_plugin(this);
}


void Plugin_SurfaceRender::update_dock_tab()
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

void Plugin_SurfaceRender::set_render_vertices(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_vertices_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_edges(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_edges_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_faces(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_faces_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_boundary(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_boundary_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_face_style(View* view, MapHandlerGen* map, MapParameters::FaceShadingStyle s)
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

void Plugin_SurfaceRender::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
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

void Plugin_SurfaceRender::set_normal_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
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

void Plugin_SurfaceRender::set_color_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_color_vbo(vbo);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_edge_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_edge_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_front_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_front_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_back_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_back_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_scale_factor(sf);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}


} // namespace plugin_surface_render

} // namespace schnapps
