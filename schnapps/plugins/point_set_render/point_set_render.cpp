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

#include <schnapps/plugins/point_set_render/point_set_render.h>
#include <schnapps/plugins/point_set_render/point_set_render_dock_tab.h>

#include <schnapps/plugins/cmap0_provider/cmap0_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

namespace plugin_point_set_render
{

Plugin_PointSetRender::Plugin_PointSetRender()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_PointSetRender::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_PointSetRender::parameters(View* view, CMap0Handler* mh)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(mh, "Try to access parameters for null map handler");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) == 0)
	{
		MapParameters& p = view_param_set[mh];
		p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(mh->map()->nb_cells<CMap0::Vertex>())));
		return p;
	}
	else
		return view_param_set[mh];
}

bool Plugin_PointSetRender::check_docktab_activation()
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

bool Plugin_PointSetRender::enable()
{
	if (setting("Auto enable on selected view").isValid())
		setting_auto_enable_on_selected_view_ = setting("Auto enable on selected view").toBool();
	else
		setting_auto_enable_on_selected_view_ = add_setting("Auto enable on selected view", true).toBool();

	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	if (setting("Auto load color attribute").isValid())
		setting_auto_load_color_attribute_ = setting("Auto load color attribute").toString();
	else
		setting_auto_load_color_attribute_ = add_setting("Auto load color attribute", "color").toString();

	dock_tab_ = new PointSetRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Point Set Render");

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));

	return true;
}

void Plugin_PointSetRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(enable_on_selected_view(Plugin*)));
}

void Plugin_PointSetRender::draw_object(View* view, Object *o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
	if (mh)
	{
		view->makeCurrent();
		MapParameters& p = parameters(view, mh);

		if (p.render_vertices_)
		{
			if (p.position_vbo_)
			{
				p.shader_point_sprite_param_->bind(proj, mv);
				mh->draw(cgogn::rendering::POINTS);
				p.shader_point_sprite_param_->release();
			}
		}
	}
}

void Plugin_PointSetRender::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_PointSetRender::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_PointSetRender::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_PointSetRender::add_linked_map(View* view, CMap0Handler* mh)
{
	set_position_vbo(view, mh, mh->vbo(setting_auto_load_position_attribute_), true);
	set_color_vbo(view, mh, mh->vbo(setting_auto_load_color_attribute_), true);

	connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
}

void Plugin_PointSetRender::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_PointSetRender::remove_linked_map(View* view, CMap0Handler* mh)
{
	disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
}

void Plugin_PointSetRender::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<CMap0Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(it.first, mh, vbo, true);
				if (!p.color_vbo_ && vbo_name == setting_auto_load_color_attribute_)
					set_color_vbo(it.first, mh, vbo, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_PointSetRender::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap0Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, mh, nullptr, true);
				if (p.color_vbo_ == vbo)
					set_color_vbo(it.first, mh, nullptr, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_PointSetRender::linked_map_bb_changed()
{
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(sender());
	const uint32 nbv = mh->map()->nb_cells<CMap0::Vertex>();

	for (auto& it : parameter_set_)
	{
		std::map<CMap0Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(nbv)));
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_PointSetRender::viewer_initialized()
{
	View* view = qobject_cast<View*>(sender());
	if (view && parameter_set_.count(view) > 0)
	{
		auto& view_param_set = parameter_set_[view];
		for (auto& p : view_param_set)
		{
			MapParameters& mp = p.second;
			mp.initialize_gl();
		}
	}
}

void Plugin_PointSetRender::enable_on_selected_view(Plugin* p)
{
	if ((this == p) && schnapps_->selected_view() && setting_auto_enable_on_selected_view_)
		schnapps_->selected_view()->link_plugin(this);
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_PointSetRender::set_position_vbo(View* view, CMap0Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_PointSetRender::set_color_vbo(View* view, CMap0Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_PointSetRender::set_render_vertices(View* view, CMap0Handler* mh, bool b, bool update_dock_tab)
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

void Plugin_PointSetRender::set_vertex_color(View* view, CMap0Handler* mh, const QColor& color, bool update_dock_tab)
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

void Plugin_PointSetRender::set_vertex_scale_factor(View* view, CMap0Handler* mh, float32 sf, bool update_dock_tab)
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


} // namespace plugin_point_set_render

} // namespace schnapps
