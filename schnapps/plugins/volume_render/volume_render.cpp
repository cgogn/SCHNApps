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

#include <volume_render.h>

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>
#endif

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_volume_render
{

MapParameters& Plugin_VolumeRender::get_parameters(View* view, MapHandlerGen* map)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(map, "Try to access parameters for null map");
	cgogn_message_assert(map->dimension() == 3, "Try to access parameters for map with dimension other than 3");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.map_ = static_cast<MapHandler<CMap3>*>(map);
		p.set_vertex_base_size(map->get_bb_diagonal_size() / (2.0f * std::sqrt(float32(map->nb_cells(Edge_Cell)))));
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_VolumeRender::check_docktab_activation()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
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

bool Plugin_VolumeRender::enable()
{
	if (get_setting("Auto enable on selected view").isValid())
		setting_auto_enable_on_selected_view_ = get_setting("Auto enable on selected view").toBool();
	else
		setting_auto_enable_on_selected_view_ = add_setting("Auto enable on selected view", true).toBool();

	if (get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new VolumeRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Volume Render");

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

#ifdef USE_TRANSPARENCY
	plugin_transparency_ = reinterpret_cast<PluginInteraction*>(schnapps_->enable_plugin("surface_render_transp"));
#endif

	return true;
}

void Plugin_VolumeRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_VolumeRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 3)
	{
		view->makeCurrent();
		MapParameters& p = get_parameters(view, map);

		if (map->is_selected_map() && p.apply_clipping_plane_)
			p.frame_manip_->draw(true, true, proj, mv, view);

		if (p.render_topology_ && p.topo_drawer_rend_)
			p.topo_drawer_rend_->draw(proj, mv, view);

		if (p.render_vertices_)
		{
			if (p.position_vbo_)
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				map->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}

		if (p.render_edges_)
		{
			if (p.position_vbo_)
			{
				if (p.volume_drawer_rend_)
					p.volume_drawer_rend_->draw_edges(proj, mv, view);
				else
				{
					p.shader_simple_color_param_->bind(proj, mv);
					map->draw(cgogn::rendering::LINES);
					p.shader_simple_color_param_->release();
				}
			}
		}

		if (p.render_faces_)
		{
			if (p.position_vbo_)
			{
				if (p.render_edges_ && p.volume_explode_factor_ > 0.995f)
					p.set_volume_explode_factor(0.995f);
				if (!p.use_transparency_ && p.volume_drawer_rend_)
					p.volume_drawer_rend_->draw_faces(proj, mv, view);
			}
		}
	}
}

bool Plugin_VolumeRender::mousePress(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		const MapParameters& p = get_parameters(view, map);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &map->get_frame());
			qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &map->get_frame());
			VEC3D A(P.x, P.y, P.z);
			VEC3D B(Q.x, Q.y, Q.z);
			p.frame_manip_->pick(event->x(), event->y(), A, B);
			view->update();
		}
	}
	return true;
}

void Plugin_VolumeRender::mouseRelease(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		const MapParameters& p = get_parameters(view, map);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			p.frame_manip_->release();
			view->update();
		}
	}
}

void Plugin_VolumeRender::mouseMove(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			bool local_manip = event->buttons() & Qt::LeftButton;
			p.frame_manip_->drag(local_manip, event->x(), event->y());
			p.update_clipping_plane();
			view->update();
		}
	}
}

void Plugin_VolumeRender::view_linked(View* view)
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

void Plugin_VolumeRender::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { remove_linked_map(view, map); }
}

void Plugin_VolumeRender::map_linked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	add_linked_map(view, map);
}

void Plugin_VolumeRender::add_linked_map(View* view, MapHandlerGen* map)
{
	if (map->dimension() == 3)
	{
		set_position_vbo(view, map, map->get_vbo(setting_auto_load_position_attribute_), true);

#ifdef USE_TRANSPARENCY
		MapParameters& p = get_parameters(view, map);
		if (p.use_transparency_)
			plugin_surface_render_transp::add_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
#endif

		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
		connect(map, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_VolumeRender::map_unlinked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	remove_linked_map(view, map);
}

void Plugin_VolumeRender::remove_linked_map(View* view, MapHandlerGen* map)
{
	if (map->dimension() == 3)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
		disconnect(map, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()));
		disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)));

#ifdef USE_TRANSPARENCY
		MapParameters& p = get_parameters(view, map);
		if (p.use_transparency_)
			plugin_surface_render_transp::remove_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
#endif

		if (check_docktab_activation())
			dock_tab_->refresh_ui();
	}
}

void Plugin_VolumeRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
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
					set_position_vbo(it.first, map, vbo, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, map, nullptr, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::linked_map_bb_changed()
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
			p.frame_manip_->set_size(map->get_bb_diagonal_size() / 12.0f);
		}
	}
}

void Plugin_VolumeRender::linked_map_connectivity_changed()
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			if (p.position_vbo_)
				p.update_volume_drawer();
		}
	}
}

void Plugin_VolumeRender::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap3::Vertex::ORBIT)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.position_vbo_ && QString::fromStdString(p.position_vbo_->name()) == attribute_name)
					p.update_volume_drawer();
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && parameter_set_.count(view) > 0)
	{
		auto& view_param_set = parameter_set_[view];
		for (auto & p : view_param_set)
		{
			MapHandlerGen* map = p.first;
			MapParameters& mp = p.second;
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				plugin_surface_render_transp::remove_tr_vol(plugin_transparency_, view, map, mp.get_transp_drawer_rend());
#endif
			mp.initialize_gl();
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				plugin_surface_render_transp::add_tr_vol(plugin_transparency_, view, map, mp.get_transp_drawer_rend());
#endif
		}
	}
}

void Plugin_VolumeRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->get_selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->get_selected_view()->link_plugin(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_VolumeRender::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_position_vbo(vbo);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_vertices(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_vertices_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_vertices(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_edges(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_edges_ = b;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_edges(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_faces(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_faces_ = b;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
		{
			if (b)
				plugin_surface_render_transp::add_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
			else
				plugin_surface_render_transp::remove_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
		}
#endif
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_faces(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_topology(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_render_topology(b);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_render_topology(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_apply_clipping_plane(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_apply_clipping_plane(b);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_apply_clipping_plane(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_vertex_color(color);
		view->update();
	}
}

void Plugin_VolumeRender::set_edge_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_edge_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_edge_color(color);
		view->update();
	}
}

void Plugin_VolumeRender::set_face_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_face_color(color);
		p.set_transparency_factor(p.get_transparency_factor());
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_face_color(color);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_scale_factor(sf);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_vertex_scale_factor(sf);
		view->update();
	}
}

void Plugin_VolumeRender::set_volume_explode_factor(View* view, MapHandlerGen* map, float32 vef, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_volume_explode_factor(vef);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_volume_explode_factor(vef);
		view->update();
	}
}

void Plugin_VolumeRender::set_transparency_enabled(View* view, MapHandlerGen* map, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_transparency_enabled(b);
#ifdef USE_TRANSPARENCY
		if (p.render_faces_)
		{
			if (b)
				plugin_surface_render_transp::add_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
			else
				plugin_surface_render_transp::remove_tr_vol(plugin_transparency_, view, map, p.get_transp_drawer_rend());
		}
#endif
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_transparency_enabled(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_transparency_factor(View* view, MapHandlerGen* map, int32 tf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_transparency_factor(tf);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_transparency_factor(tf);
		view->update();
	}
}

} // namespace plugin_volume_render

} // namespace schnapps
