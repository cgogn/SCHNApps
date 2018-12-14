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

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>
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

MapParameters& Plugin_SurfaceRenderScalar::parameters(View* view, CMap2Handler* mh)
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

bool Plugin_SurfaceRenderScalar::check_docktab_activation()
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

bool Plugin_SurfaceRenderScalar::enable()
{
	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
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

void Plugin_SurfaceRenderScalar::draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
	{
		view->makeCurrent();
		const MapParameters& p = parameters(view, mh);

		if (p.position_vbo_ && p.scalar_vbo_)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0f, 1.0f);
			p.shader_scalar_per_vertex_param_->bind(proj, mv);
			mh->draw(cgogn::rendering::TRIANGLES);
			p.shader_scalar_per_vertex_param_->release();
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
}

void Plugin_SurfaceRenderScalar::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_SurfaceRenderScalar::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_SurfaceRenderScalar::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_SurfaceRenderScalar::add_linked_map(View* view, CMap2Handler* mh)
{
	set_position_vbo(view, mh, mh->vbo(setting_auto_load_position_attribute_), true);

	connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
	connect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
}

void Plugin_SurfaceRenderScalar::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_SurfaceRenderScalar::remove_linked_map(View*, CMap2Handler* mh)
{
	disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_added(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(linked_map_vbo_removed(cgogn::rendering::VBO*)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
	disconnect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, QString)));
}

void Plugin_SurfaceRenderScalar::linked_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3)
	{
		CMap2Handler* mh = static_cast<CMap2Handler*>(sender());

		const QString vbo_name = QString::fromStdString(vbo->name());
		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				const MapParameters& p = view_param_set[mh];
				if (!p.position_vbo_ && vbo_name == setting_auto_load_position_attribute_)
					set_position_vbo(it.first, mh, vbo, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderScalar::linked_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	if (vbo->vector_dimension() == 3 || vbo->vector_dimension() == 1)
	{
		CMap2Handler* mh = static_cast<CMap2Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_vbo_ == vbo)
					set_position_vbo(it.first, mh, nullptr, true);
				if (p.scalar_vbo_ == vbo)
					set_scalar_vbo(it.first, mh, nullptr, true);
			}
		}

		for (View* view : mh->linked_views())
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
		CMap2Handler* mh = static_cast<CMap2Handler*>(sender());

		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.scalar_vbo_ && p.auto_update_min_max_ && QString::fromStdString(p.scalar_vbo_->name()) == attribute_name)
					update_min_max(it.first, mh, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceRenderScalar::viewer_initialized()
{
	View* view = qobject_cast<View*>(sender());
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

void Plugin_SurfaceRenderScalar::set_position_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
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

void Plugin_SurfaceRenderScalar::set_scalar_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_scalar_vbo(vbo);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_scalar_vbo(vbo);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_color_map(View* view, CMap2Handler* mh, cgogn::rendering::ShaderScalarPerVertex::ColorMap cm, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_color_map(cm);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_color_map(cm);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_auto_update_min_max(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_auto_update_min_max(b);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_auto_update_min_max(b);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_scalar_min(View* view, CMap2Handler* mh, double d, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_scalar_min(d);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_scalar_min(d);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_scalar_max(View* view, CMap2Handler* mh, double d, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_scalar_max(d);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_scalar_max(d);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_expansion(View* view, CMap2Handler* mh, int32 i, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_expansion(i);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_expansion(i);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_show_iso_lines(View* view, CMap2Handler* mh, bool b, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_show_iso_lines(b);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_show_iso_lines(b);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::set_nb_iso_levels(View* view, CMap2Handler* mh, int32 i, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_nb_iso_levels(i);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_nb_iso_levels(i);
		view->update();
	}
}

void Plugin_SurfaceRenderScalar::update_min_max(View* view, CMap2Handler* mh, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.update_min_max();
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
		{
			dock_tab_->set_scalar_min(p.scalar_min());
			dock_tab_->set_scalar_max(p.scalar_max());
		}
		view->update();
	}
}

} // namespace plugin_surface_render_scalar

} // namespace schnapps
