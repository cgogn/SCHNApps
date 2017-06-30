/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Render                                                         *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* Inspired by the surface render plugin                                        *
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

#include "volume_render.h"

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>

#include <cgogn/geometry/algos/selection.h>


namespace schnapps
{

namespace plugin_volume_render
{

MapParameters& Plugin_VolumeRender::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.map_ = static_cast<MapHandler<CMap3>*>(map);
		if (map->get_bb().is_initialized())
		{
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2.0f * std::sqrt(float32(map->nb_cells(Edge_Cell)))));
			p.frame_manip_->set_size(map->get_bb_diagonal_size() / 12.0f);
			p.frame_manip_->set_position(map->get_bb().max());
			p.frame_manip_->z_plane_param(QColor(200,200,200), 0.0f, 0.0f, 3.0f);
		}
		return p;
	}
	else
		return view_param_set[map];
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

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

	update_dock_tab();

#ifdef USE_TRANSP
	plug_transp_ = reinterpret_cast<PluginInteraction*>(schnapps_->enable_plugin("surface_render_transp"));
#endif

	return true;
}

void Plugin_VolumeRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
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

		if (p.render_topology_)
			if (p.topo_drawer_rend_)
				p.topo_drawer_rend_->draw(proj,mv,view);

		if (p.render_vertices_)
		{
			if (p.get_position_vbo())
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				map->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}

		if (p.render_edges_)
		{
			if (p.get_position_vbo())
			{
				if(p.volume_drawer_rend_)
				{
					p.volume_drawer_rend_->draw_edges(proj, mv, view);
				}
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
			if (p.get_position_vbo())
			{
				if (p.render_edges_ && (p.volume_explode_factor_>0.995f))
					p.set_volume_explode_factor(0.995f);
				if (!p.use_transparency_ && p.volume_drawer_rend_)
					p.volume_drawer_rend_->draw_faces(proj, mv, view);
			}
		}
	}
}

void Plugin_VolumeRender::mousePress(View* view, QMouseEvent* event)
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
			p.set_apply_clipping_plane(true);
			view->update();
		}
	}
}


void Plugin_VolumeRender::view_linked(View* view)
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

void Plugin_VolumeRender::view_unlinked(View* view)
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

void Plugin_VolumeRender::connectivity_changed(MapHandlerGen* map)
{
	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			CMap3Handler* mh3 = static_cast<CMap3Handler*>(map);
			if (!p.position_vbo_) return;
			auto pos_attr = mh3->get_attribute<VEC3, CMap3::Vertex::ORBIT>(QString::fromStdString(p.position_vbo_->name()));
			if (pos_attr.is_valid())
			{
				if (!p.use_transparency_)
				{
					p.volume_drawer_->update_face<VEC3>(*mh3->get_map(), pos_attr);
					p.volume_drawer_->update_edge<VEC3>(*mh3->get_map(), pos_attr);
				}
#ifdef USE_TRANSP
				else {
					p.volume_transparency_drawer_->update_face<VEC3>(*mh3->get_map(), pos_attr);
				}
#endif
				p.topo_drawer_->update<VEC3>(*mh3->get_map(),pos_attr);
			}
		}
	}
}

void Plugin_VolumeRender::map_linked(View* view, MapHandlerGen* map)
{
	update_dock_tab();

	if (map->dimension() == 3)
	{
		set_position_vbo(view->get_name(), map->get_name(), setting_auto_load_position_attribute_);
		MapParameters& p = get_parameters(view, map);
#ifdef USE_TRANSP
		if (p.use_transparency_)
				plugin_surface_render_transp::add_tr_vol(plug_transp_,view,map,p.get_transp_drawer_rend());
#endif
		connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
		connect(map, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit,QString)), this, SLOT(linked_attribute_changed(cgogn::Orbit,QString)), Qt::UniqueConnection);
	}
}

void Plugin_VolumeRender::map_unlinked(View* view, MapHandlerGen* map)
{
	update_dock_tab();

	if (map->dimension() == 3)
	{
		disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
		disconnect(map, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()));
		disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit,QString)), this, SLOT(linked_attribute_changed(cgogn::Orbit,QString)));

		MapParameters& p = get_parameters(view, map);
#ifdef USE_TRANSP
		if (p.use_transparency_)
			plugin_surface_render_transp::remove_tr_vol(plug_transp_,view,map,p.get_transp_drawer_rend());
#endif
	}
}

void Plugin_VolumeRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (map && map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			const QString vbo_name = QString::fromStdString(vbo->name());
			dock_tab_->add_position_vbo(vbo_name);
			View* view = schnapps_->get_selected_view();
			if (view)
			{
				if (!get_parameters(view, map).get_position_vbo() && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(view->get_name(), map->get_name(), vbo_name);
			}
		}
	}
}

void Plugin_VolumeRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());

	if (map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
	}

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			if (p.get_position_vbo() == vbo)
				p.set_position_vbo(nullptr);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_VolumeRender::linked_map_bb_changed()
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());
	if (!map)
		return;

	const uint32 nbe = map->nb_cells(Edge_Cell);

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2.0f * std::sqrt(float(nbe))));
			p.frame_manip_->set_size(map->get_bb_diagonal_size() / 12.0f);
		}
	}
}

void Plugin_VolumeRender::linked_map_connectivity_changed()
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());
	if (!map) return;

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			CMap3Handler* mh3 = static_cast<CMap3Handler*>(map);
			map->update_vbo(QString::fromStdString(p.position_vbo_->name()));
			auto pos_attr = mh3->get_attribute<VEC3, CMap3::Vertex::ORBIT>(QString::fromStdString(p.position_vbo_->name()));
			if (pos_attr.is_valid())
			{
				if (!p.use_transparency_)
				{
					p.volume_drawer_->update_edge<VEC3>(*mh3->get_map(), pos_attr);
					p.volume_drawer_->update_face<VEC3>(*mh3->get_map(), pos_attr);
				}
#ifdef USE_TRANSP
				else {
					p.volume_transparency_drawer_->update_face<VEC3>(*mh3->get_map(), pos_attr);
				}
#endif
				p.topo_drawer_->update<VEC3>(*mh3->get_map(),pos_attr);
			}
		}
	}
}

void Plugin_VolumeRender::linked_attribute_changed(cgogn::Orbit, QString)
{
	MapHandlerGen* map = dynamic_cast<MapHandlerGen*>(sender());
	if (map)
		this->connectivity_changed(map);
}

void Plugin_VolumeRender::viewer_initialized()
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
			plugin_surface_render_transp::remove_tr_vol(plug_transp_, view, map, mp.get_transp_drawer_rend());
	#endif
			mp.initialize_gl();
	#ifdef USE_TRANSP
			plugin_surface_render_transp::add_tr_vol(plug_transp_, view, map, mp.get_transp_drawer_rend());
	#endif
		}
	}
	update_dock_tab();
}

void Plugin_VolumeRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->get_selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->get_selected_view()->link_plugin(this);
}

void Plugin_VolumeRender::update_dock_tab()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();
	if (view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3 && map->get_bb().is_initialized())
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

void Plugin_VolumeRender::set_render_vertices(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_vertices_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_edges(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_edges_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_faces(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_faces_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_boundary(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.render_boundary_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_vbo(vbo);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_edge_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_edge_color(color);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_face_color(View* view, MapHandlerGen* map, const QColor& color)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_face_color(color);
		p.set_transparency_factor(p.get_transparency_factor());
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_scale_factor(sf);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_volume_explode_factor(View* view, MapHandlerGen* map, float32 vef)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.set_volume_explode_factor(vef);
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

void Plugin_VolumeRender::set_apply_clipping_plane(View* view, MapHandlerGen* map, bool b)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 3)
	{
		MapParameters& p = get_parameters(view, map);
		p.apply_clipping_plane_ = b;
		if (view->is_selected_view() && map->is_selected_map())
			dock_tab_->update_map_parameters(map, p);
		view->update();
	}
}

MapParameters::MapParameters() :
	position_vbo_(nullptr),
	plane_clipping_(0., 0., 0., 0.),
#ifdef USE_TRANSP
	volume_transparency_drawer_(nullptr),
	volume_transparency_drawer_rend_(nullptr),
#endif
	apply_clipping_plane_(false),
	render_vertices_(false),
	render_edges_(false),
	render_faces_(true),
	render_boundary_(false),
	render_topology_(false),
	use_transparency_(false)
{
	transparency_factor_ = 127;
	vertex_color_ = QColor(190, 85, 168);
	edge_color_ = QColor(0, 0, 0);
	face_color_ = QColor(85, 168, 190);
	volume_explode_factor_ = 0.8f;
	vertex_scale_factor_ = 1;
	vertex_base_size_ = 1;

	initialize_gl();
}

void MapParameters::set_position_vbo(cgogn::rendering::VBO* v)
{
	auto old = position_vbo_;
	position_vbo_ = v;
	if (position_vbo_ && position_vbo_->vector_dimension() == 3)
	{
		shader_simple_color_param_->set_position_vbo(position_vbo_);
		shader_point_sprite_param_->set_position_vbo(position_vbo_);

		auto pos_attr = map_->get_attribute<VEC3, CMap3::Vertex::ORBIT>(QString::fromStdString(position_vbo_->name()));
		if (pos_attr.is_valid())
		{
			if (!use_transparency_)
			{
				volume_drawer_->update_face<VEC3>(*map_->get_map(), pos_attr);
				volume_drawer_->update_edge<VEC3>(*map_->get_map(), pos_attr);
			}
#ifdef USE_TRANSP
			else {
				volume_transparency_drawer_->update_face<VEC3>(*map_->get_map(), pos_attr);
			}
#endif
			topo_drawer_->update<VEC3>(*map_->get_map(),pos_attr);
		}
	} else
		position_vbo_ = old;
}

void MapParameters::initialize_gl()
{
	shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
	shader_simple_color_param_->color_ = edge_color_;

	shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
	shader_point_sprite_param_->color_ = vertex_color_;
	shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

	volume_drawer_ = cgogn::make_unique<cgogn::rendering::VolumeDrawer>();
	volume_drawer_rend_ = volume_drawer_->generate_renderer();

	topo_drawer_ =  cgogn::make_unique<cgogn::rendering::TopoDrawer>();
	topo_drawer_rend_ = topo_drawer_->generate_renderer();


#ifdef USE_TRANSP
	{
//		if (volume_transparency_drawer_ == nullptr)
			volume_transparency_drawer_ = cgogn::make_unique<cgogn::rendering::VolumeTransparencyDrawer>();
		volume_transparency_drawer_rend_ = volume_transparency_drawer_->generate_renderer();
		volume_transparency_drawer_rend_->set_explode_volume(volume_explode_factor_);
		volume_transparency_drawer_rend_->set_lighted(true);
	}
#endif
	frame_manip_ = cgogn::make_unique<cgogn::rendering::FrameManipulator>();

	volume_drawer_rend_->set_explode_volume(volume_explode_factor_);
	topo_drawer_->set_explode_volume(volume_explode_factor_);

	set_transparency_factor(transparency_factor_);
	set_vertex_color(vertex_color_);
	set_edge_color(edge_color_);
	set_face_color(face_color_);
	set_volume_explode_factor(volume_explode_factor_);
	set_vertex_scale_factor(vertex_scale_factor_);
	set_vertex_base_size(vertex_base_size_);

	set_position_vbo(position_vbo_);
}


} // namespace plugin_volume_render

} // namespace schnapps
