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

#include <schnapps/plugins/volume_render/volume_render.h>
#include <schnapps/plugins/volume_render/volume_render_dock_tab.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/render_transparency/render_transparency.h>
#endif

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

namespace plugin_volume_render
{

Plugin_VolumeRender::Plugin_VolumeRender()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_VolumeRender::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_VolumeRender::parameters(View* view, CMap3Handler* mh)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(mh, "Try to access parameters for null map");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) == 0)
	{
		MapParameters& p = view_param_set[mh];
		p.mh_ = mh;
		p.set_vertex_base_size(mh->bb_diagonal_size() / (2.0f * std::sqrt(mh->map()->nb_cells<CMap3::Edge>())));
		return p;
	}
	else
		return view_param_set[mh];
}

bool Plugin_VolumeRender::check_docktab_activation()
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

bool Plugin_VolumeRender::enable()
{
	if (setting("Auto enable on selected view").isValid())
		setting_auto_enable_on_selected_view_ = setting("Auto enable on selected view").toBool();
	else
		setting_auto_enable_on_selected_view_ = add_setting("Auto enable on selected view", true).toBool();

	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new VolumeRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Volume Render");

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

#ifdef USE_TRANSPARENCY
	plugin_transparency_ = qobject_cast<plugin_render_transparency::Plugin_RenderTransparency*>(schnapps_->enable_plugin(plugin_render_transparency::Plugin_RenderTransparency::plugin_name()));
#endif

	return true;
}

void Plugin_VolumeRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_VolumeRender::draw_object(View* view, Object *o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
	if (mh)
	{
		view->makeCurrent();
		MapParameters& p = parameters(view, mh);

		if (mh == dock_tab_->selected_map() && p.apply_clipping_plane_)
			p.frame_manip_->draw(true, true, proj, mv);

		if (p.render_topology_ && p.topo_drawer_rend_)
			p.topo_drawer_rend_->draw(proj, mv);

		if (p.render_vertices_)
		{
			if (p.position_vbo_)
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				mh->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}

		if (p.render_edges_)
		{
			if (p.position_vbo_)
			{
				if (p.volume_drawer_rend_)
					p.volume_drawer_rend_->draw_edges(proj, mv);
				else
				{
					p.shader_simple_color_param_->bind(proj, mv);
					mh->draw(cgogn::rendering::LINES);
					p.shader_simple_color_param_->release();
				}
			}
		}

		if (p.render_faces_)
		{
			if (p.position_vbo_)
			{
                if(p.render_color_per_volumes_)
                {
                    if (p.render_edges_ && p.volume_explode_factor_ > 0.995f)
                        p.set_volume_explode_factor(0.995f);
                    if (!p.use_transparency_ && p.volume_drawer_rend_)
                        p.volume_drawer_color_rend_->draw_faces(proj, mv);
                }
                else
                {
                    if (p.render_edges_ && p.volume_explode_factor_ > 0.995f)
                        p.set_volume_explode_factor(0.995f);
                    if (!p.use_transparency_ && p.volume_drawer_rend_)
                        p.volume_drawer_rend_->draw_faces(proj, mv);
                 }
             }
		}
	}
}

bool Plugin_VolumeRender::mousePress(View* view, QMouseEvent* event)
{
	CMap3Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		const MapParameters& p = parameters(view, mh);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &mh->frame());
			qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &mh->frame());
			VEC3D A(P.x, P.y, P.z);
			VEC3D B(Q.x, Q.y, Q.z);
			p.frame_manip_->pick(event->x(), event->y(), A, B);
			view->update();
		}
	}
	return true;
}

bool Plugin_VolumeRender::mouseRelease(View* view, QMouseEvent* event)
{
	CMap3Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		const MapParameters& p = parameters(view, mh);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			p.frame_manip_->release();
			view->update();
		}
	}
	return true;
}

bool Plugin_VolumeRender::mouseMove(View* view, QMouseEvent* event)
{
	CMap3Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		if (p.apply_clipping_plane_ && event->modifiers() & Qt::ShiftModifier)
		{
			bool local_manip = event->buttons() & Qt::LeftButton;
			p.frame_manip_->drag(local_manip, event->x(), event->y());
			p.update_clipping_plane();
			view->update();
		}
	}
	return true;
}

void Plugin_VolumeRender::view_linked(View* view)
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
		CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_VolumeRender::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_VolumeRender::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_VolumeRender::add_linked_map(View* view, CMap3Handler* mh)
{
	set_position_vbo(view, mh, mh->vbo(setting_auto_load_position_attribute_), true);

#ifdef USE_TRANSPARENCY
	MapParameters& p = parameters(view, mh);
	if (p.use_transparency_)
		plugin_transparency_->add_tr_vol(view, mh, p.transp_drawer_rend());
#endif

	connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
	connect(mh, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()), Qt::UniqueConnection);
	connect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
}

void Plugin_VolumeRender::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_VolumeRender::remove_linked_map(View* view, CMap3Handler* mh)
{
	disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
	disconnect(mh, SIGNAL(connectivity_changed()), this, SLOT(linked_map_connectivity_changed()));
	disconnect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)));

#ifdef USE_TRANSPARENCY
	MapParameters& p = parameters(view, mh);
	if (p.use_transparency_)
		plugin_transparency_->remove_tr_vol(view, mh, p.transp_drawer_rend());
#endif
}

void Plugin_VolumeRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap3Handler* mh = qobject_cast<CMap3Handler*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<CMap3Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(it.first, mh, vbo, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap3Handler* mh = qobject_cast<CMap3Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap3Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, mh, nullptr, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::linked_map_bb_changed()
{
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(sender());
	const uint32 nbe = mh->map()->nb_cells<CMap3::Edge>();

	for (auto& it : parameter_set_)
	{
		std::map<CMap3Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(nbe)));
			p.frame_manip_->set_size(mh->bb_diagonal_size() / 12.0f);
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_VolumeRender::linked_map_connectivity_changed()
{
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(sender());

	for (auto& it : parameter_set_)
	{
		std::map<CMap3Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			if (p.position_vbo_)
				p.update_volume_drawer();
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_VolumeRender::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap3::Vertex::ORBIT)
	{
		CMap3Handler* mh = static_cast<CMap3Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap3Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_vbo_ && QString::fromStdString(p.position_vbo_->name()) == attribute_name)
					p.update_volume_drawer();
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_VolumeRender::viewer_initialized()
{
	View* view = qobject_cast<View*>(sender());
	if (view && parameter_set_.count(view) > 0)
	{
		auto& view_param_set = parameter_set_[view];
		for (auto & p : view_param_set)
		{
			CMap3Handler* mh = p.first;
			MapParameters& mp = p.second;
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				plugin_transparency_->remove_tr_vol(view, mh, mp.transp_drawer_rend());
#endif
			mp.initialize_gl();
#ifdef USE_TRANSPARENCY
			if (mp.use_transparency_)
				plugin_transparency_->add_tr_vol(view, mh, mp.transp_drawer_rend());
#endif
		}
	}
}

void Plugin_VolumeRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->selected_view()->link_plugin(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_VolumeRender::set_color_per_volume(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
{
    if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
    {
        MapParameters& p = parameters(view, mh);
        p.set_render_color_per_volume(b);
//        if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
//            dock_tab_->set_color_per_volume(b);
        view->update();
    }
}

void Plugin_VolumeRender::set_volume_attribute(View* view, CMap3Handler* mh, const QString& attrib, bool update_dock_tab)
{
    if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
    {
        MapParameters& p = parameters(view, mh);
        p.set_volume_attribute(attrib);
//        if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
//            dock_tab_->set_color_per_volume(b);
        view->update();
    }
}

void Plugin_VolumeRender::set_color_map(View* view, CMap3Handler* mh, const QString& color_map, bool update_dock_tab)
{
    if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
    {
        MapParameters& p = parameters(view, mh);
        p.set_color_map(color_map);
//        if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
//            dock_tab_->set_color_per_volume(b);
        view->update();
    }
}

void Plugin_VolumeRender::set_position_vbo(View* view, CMap3Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_VolumeRender::set_render_vertices(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
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

void Plugin_VolumeRender::set_render_edges(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
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

void Plugin_VolumeRender::set_render_faces(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.render_faces_ = b;
#ifdef USE_TRANSPARENCY
		if (p.use_transparency_)
		{
			if (b)
				plugin_transparency_->add_tr_vol(view, mh, p.transp_drawer_rend());
			else
				plugin_transparency_->remove_tr_vol(view, mh, p.transp_drawer_rend());
		}
#endif
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_faces(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_render_topology(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_render_topology(b);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_render_topology(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_apply_clipping_plane(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_apply_clipping_plane(b);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_apply_clipping_plane(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab)
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

void Plugin_VolumeRender::set_edge_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab)
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

void Plugin_VolumeRender::set_face_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_face_color(color);
		p.set_transparency_factor(p.transparency_factor());
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_face_color(color);
		view->update();
	}
}

void Plugin_VolumeRender::set_vertex_scale_factor(View* view, CMap3Handler* mh, float32 sf, bool update_dock_tab)
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

void Plugin_VolumeRender::set_volume_explode_factor(View* view, CMap3Handler* mh, float32 vef, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_volume_explode_factor(vef);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_volume_explode_factor(vef);
		view->update();
	}
}

void Plugin_VolumeRender::set_transparency_enabled(View* view, CMap3Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_transparency_enabled(b);
#ifdef USE_TRANSPARENCY
		if (p.render_faces_)
		{
			if (b)
				plugin_transparency_->add_tr_vol(view, mh, p.transp_drawer_rend());
			else
				plugin_transparency_->remove_tr_vol(view, mh, p.transp_drawer_rend());
		}
#endif
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_transparency_enabled(b);
		view->update();
	}
}

void Plugin_VolumeRender::set_transparency_factor(View* view, CMap3Handler* mh, int32 tf, bool update_dock_tab)
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

} // namespace plugin_volume_render

} // namespace schnapps
