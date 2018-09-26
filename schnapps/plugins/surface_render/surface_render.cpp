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

#include <schnapps/plugins/surface_render/surface_render.h>
#include <schnapps/plugins/surface_render/surface_render_dock_tab.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/surface_render_transp/surface_render_transp.h>
#endif

namespace schnapps
{

namespace plugin_surface_render
{

Plugin_SurfaceRender::Plugin_SurfaceRender()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceRender::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_SurfaceRender::parameters(View* view, CMap2Handler* mh)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(mh, "Try to access parameters for null map handler");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) == 0)
	{
		MapParameters& p = view_param_set[mh];
		p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(mh->map()->nb_cells<CMap2::Edge>())));
		return p;
	}
	else
		return view_param_set[mh];
}

bool Plugin_SurfaceRender::check_docktab_activation()
{
	View* view = schnapps_->selected_view();

	if (view && view->is_linked_to_plugin(this))
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
	if (setting("Auto enable on selected view").isValid())
		setting_auto_enable_on_selected_view_ = setting("Auto enable on selected view").toBool();
	else
		setting_auto_enable_on_selected_view_ = add_setting("Auto enable on selected view", true).toBool();

	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	if (setting("Auto load normal attribute").isValid())
		setting_auto_load_normal_attribute_ = setting("Auto load normal attribute").toString();
	else
		setting_auto_load_normal_attribute_ = add_setting("Auto load normal attribute", "normal").toString();

	if (setting("Auto load color attribute").isValid())
		setting_auto_load_color_attribute_ = setting("Auto load color attribute").toString();
	else
		setting_auto_load_color_attribute_ = add_setting("Auto load color attribute", "color").toString();

	dock_tab_ = new SurfaceRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render");

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

#ifdef USE_TRANSPARENCY
	plugin_transparency_ = reinterpret_cast<plugin_surface_render_transp::Plugin_SurfaceRenderTransp*>(schnapps_->enable_plugin(plugin_surface_render_transp::Plugin_SurfaceRenderTransp::plugin_name()));
#endif

	return true;
}

void Plugin_SurfaceRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_SurfaceRender::draw_object(View* view, Object *o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap2Handler* map = dynamic_cast<CMap2Handler*>(o);
	if (map)
	{
		view->makeCurrent();
		MapParameters& p = parameters(view, map);

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

	connect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_SurfaceRender::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_SurfaceRender::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_SurfaceRender::add_linked_map(View* view, CMap2Handler* mh)
{
	set_position_vbo(view, mh, mh->vbo(setting_auto_load_position_attribute_), true);
	set_normal_vbo(view, mh, mh->vbo(setting_auto_load_normal_attribute_), true);
	set_color_vbo(view, mh, mh->vbo(setting_auto_load_color_attribute_), true);

#ifdef USE_TRANSPARENCY
	MapParameters& p = parameters(view, mh);
	if (p.use_transparency_)
		add_transparency(view, mh);
#endif

	connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_SurfaceRender::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_SurfaceRender::remove_linked_map(View* view, CMap2Handler* mh)
{
	disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));

#ifdef USE_TRANSPARENCY
	MapParameters& p = parameters(view, mh);
	if (p.use_transparency_)
		remove_transparency(view, mh);
#endif

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_SurfaceRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(it.first, mh, vbo, true);
				if (!p.normal_vbo_ && vbo_name == setting_auto_load_normal_attribute_)
					set_normal_vbo(it.first, mh, vbo, true);
				if (!p.color_vbo_ && vbo_name == setting_auto_load_color_attribute_)
					set_color_vbo(it.first, mh, vbo, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, mh, nullptr, true);
				if (p.normal_vbo_ == vbo)
					set_normal_vbo(it.first, mh, nullptr, true);
				if (p.color_vbo_ == vbo)
					set_color_vbo(it.first, mh, nullptr, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRender::linked_map_bb_changed()
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());
	const uint32 nbe = mh->map()->nb_cells<CMap2::Edge>();

	for (auto& it : parameter_set_)
	{
		std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(nbe)));
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_SurfaceRender::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && parameter_set_.count(view) > 0)
	{
		auto& view_param_set = parameter_set_[view];
		for (auto& p : view_param_set)
		{
			CMap2Handler* mh = p.first;
			MapParameters& mp = p.second;
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				remove_transparency(view, mh);
#endif
			mp.initialize_gl();
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				add_transparency(view, mh);
#endif
		}
	}
}

#ifdef USE_TRANSPARENCY
void Plugin_SurfaceRender::add_transparency(View* view, CMap2Handler* mh)
{
	const MapParameters& p = parameters(view, mh);
	if (p.face_style_ == MapParameters::FLAT)
		plugin_transparency_->add_tr_flat(view, mh, p.transp_flat_param());
	else if (p.face_style_ == MapParameters::PHONG)
		plugin_transparency_->add_tr_phong(view, mh, p.transp_phong_param());
}

void Plugin_SurfaceRender::remove_transparency(View* view, CMap2Handler* mh)
{
	const MapParameters& p = parameters(view, mh);
	if (p.face_style_ == MapParameters::FLAT)
		plugin_transparency_->remove_tr_flat(view, mh, p.transp_flat_param());
	else if (p.face_style_ == MapParameters::PHONG)
		plugin_transparency_->remove_tr_phong(view, mh, p.transp_phong_param());
}

void Plugin_SurfaceRender::change_transparency(View* view, CMap2Handler* mh)
{
	const MapParameters& p = parameters(view, mh);
	if (p.face_style_ == MapParameters::FLAT)
	{
		plugin_transparency_->remove_tr_phong(view, mh, p.transp_phong_param());
		plugin_transparency_->add_tr_flat(view, mh, p.transp_flat_param());
	}
	else if (p.face_style_ == MapParameters::PHONG)
	{
		plugin_transparency_->remove_tr_flat(view, mh, p.transp_flat_param());
		plugin_transparency_->add_tr_phong(view, mh, p.transp_phong_param());
	}
}
#endif

void Plugin_SurfaceRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->selected_view()->link_plugin(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceRender::set_position_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_position_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_position_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRender::set_normal_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_normal_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_normal_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRender::set_color_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_color_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_color_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_vertices(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.render_vertices_ = b;
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_vertices(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_edges(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.render_edges_ = b;
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_edges(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_faces(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.render_faces_ = b;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
		{
			if (b)
				add_transparency(view, mh);
			else
				remove_transparency(view, mh);
		}
#endif
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_faces(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_backfaces(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_render_backfaces(b);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_backfaces(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_face_style(View* view, CMap2Handler* mh, MapParameters::FaceShadingStyle s, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.face_style_ = s;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
			change_transparency(view, mh);
#endif
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_face_style(s);
		view->update();
	}
}

void Plugin_SurfaceRender::set_render_boundary(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.render_boundary_ = b;
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_boundary(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_vertex_color(color);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_vertex_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_edge_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_edge_color(color);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_edge_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_front_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_front_color(color);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_front_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_back_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_back_color(color);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_back_color(color);
		view->update();
	}
}

void Plugin_SurfaceRender::set_vertex_scale_factor(View* view, CMap2Handler* mh, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_vertex_scale_factor(sf);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_vertex_scale_factor(sf);
		view->update();
	}
}

void Plugin_SurfaceRender::set_transparency_enabled(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_transparency_enabled(b);
#ifdef USE_TRANSPARENCY
		if (p.render_faces_)
		{
			if (b)
				add_transparency(view, mh);
			else
				remove_transparency(view, mh);
		}
#endif
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_transparency_enabled(b);
		view->update();
	}
}

void Plugin_SurfaceRender::set_transparency_factor(View* view, CMap2Handler* mh, int32 tf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_transparency_factor(tf);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_transparency_factor(tf);
		view->update();
	}
}

} // namespace plugin_surface_render

} // namespace schnapps
