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

#include <schnapps/plugins/surface_render_vector/surface_render_vector.h>
#include <schnapps/plugins/surface_render_vector/surface_render_vector_dock_tab.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

namespace plugin_surface_render_vector
{

Plugin_SurfaceRenderVector::Plugin_SurfaceRenderVector()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceRenderVector::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_SurfaceRenderVector::parameters(View* view, CMap2Handler* mh)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(mh, "Try to access parameters for null map");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) == 0)
	{
		MapParameters& p = view_param_set[mh];
		p.mh_ = static_cast<CMap2Handler*>(mh);
		return p;
	}
	else
		return view_param_set[mh];
}

bool Plugin_SurfaceRenderVector::check_docktab_activation()
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

bool Plugin_SurfaceRenderVector::enable()
{
	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new SurfaceRenderVector_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Vector");

	return true;
}

void Plugin_SurfaceRenderVector::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

void Plugin_SurfaceRenderVector::draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
	{
		view->makeCurrent();
		const MapParameters& p = parameters(view, mh);

		if (p.position_vbo_)
		{
			for (auto& param : p.shader_params())
			{
				param->bind(proj, mv);
				mh->draw(cgogn::rendering::POINTS);
				param->release();
			}
		}
	}
}

void Plugin_SurfaceRenderVector::view_linked(View* view)
{
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

void Plugin_SurfaceRenderVector::view_unlinked(View* view)
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

void Plugin_SurfaceRenderVector::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_SurfaceRenderVector::add_linked_map(View* view, CMap2Handler* mh)
{
	set_position_vbo(view, mh, mh->vbo(setting_auto_load_position_attribute_), true);

	connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
}

void Plugin_SurfaceRenderVector::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_SurfaceRenderVector::remove_linked_map(View*, CMap2Handler* mh)
{
	disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
}

void Plugin_SurfaceRenderVector::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
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
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderVector::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
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
				if (p.vector_vbo_index(vbo) != UINT32_MAX)
					remove_vector_vbo(it.first, mh, vbo, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderVector::linked_map_bb_changed()
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	for (auto& it : parameter_set_)
	{
		std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0)
		{
			MapParameters& p = view_param_set[mh];
			for (uint32 i = 0, size = p.vector_scale_factor_list_.size(); i < size; ++i)
				set_vector_scale_factor(it.first, mh, p.vector_vbo(i), p.vector_scale_factor_list_[i], true);
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_SurfaceRenderVector::viewer_initialized()
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

void Plugin_SurfaceRenderVector::set_position_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_SurfaceRenderVector::add_vector_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.add_vector_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->add_vector_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRenderVector::remove_vector_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.remove_vector_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->remove_vector_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRenderVector::set_vector_scale_factor(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		const uint32 index = p.vector_vbo_index(vbo);
		if (index != UINT32_MAX)
		{
			p.set_vector_scale_factor(index, sf);
			if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
				dock_tab_->set_vector_size(vbo, sf);
			view->update();
		}
	}
}

void Plugin_SurfaceRenderVector::set_vector_color(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		const uint32 index = p.vector_vbo_index(vbo);
		if (index != UINT32_MAX)
		{
			p.set_vector_color(index, color);
			if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
				dock_tab_->set_vector_color(vbo, color);
			view->update();
		}
	}
}

} // namespace plugin_surface_render_vector

} // namespace schnapps
