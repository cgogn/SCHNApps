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

#include <surface_render_vector.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

MapParameters& Plugin_SurfaceRenderVector::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	return parameter_set_[view][map];
}

bool Plugin_SurfaceRenderVector::enable()
{
//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(m_schnapps->getStaticPointers());

	dock_tab_ = new SurfaceRenderVector_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Vector");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	foreach(MapHandlerGen* map, schnapps_->get_map_set().values())
		map_added(map);

	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map)
	{
		View* view = schnapps_->get_selected_view();
		const MapParameters& p = get_parameters(view, map);
		dock_tab_->update_map_parameters(map, p);
	}

	return true;
}

void Plugin_SurfaceRenderVector::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	foreach(MapHandlerGen* map, schnapps_->get_map_set().values())
		map_removed(map);
}

void Plugin_SurfaceRenderVector::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	const MapParameters& p = get_parameters(view, map);

	if (p.get_position_vbo())
	{
		for (auto& param : p.get_shader_params())
		{
			param->bind(proj, mv);
			map->draw(cgogn::rendering::POINTS);
			param->release();
		}
	}
}

void Plugin_SurfaceRenderVector::selected_view_changed(View* old, View* cur)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	const MapParameters& p = get_parameters(cur, map);
	dock_tab_->update_map_parameters(map, p);
}

void Plugin_SurfaceRenderVector::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	View* view = schnapps_->get_selected_view();
	const MapParameters& p = get_parameters(view, cur);
	dock_tab_->update_map_parameters(cur, p);
}

void Plugin_SurfaceRenderVector::map_added(MapHandlerGen *map)
{
	connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_SurfaceRenderVector::map_removed(MapHandlerGen *map)
{
	disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_SurfaceRenderVector::schnapps_closing()
{

}

void Plugin_SurfaceRenderVector::vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == schnapps_->get_selected_map())
	{
		if(vbo->vector_dimension() == 3)
		{
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->add_vector_vbo(QString::fromStdString(vbo->get_name()));
		}
	}
}

void Plugin_SurfaceRenderVector::vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == schnapps_->get_selected_map())
	{
		if(vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->remove_vector_vbo(QString::fromStdString(vbo->get_name()));
		}
	}

	QSet<View*> views_to_update;

	for (auto& it : parameter_set_)
	{
		View* view = it.first;
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		MapParameters& map_param = view_param_set[map];
		if(map_param.get_position_vbo() == vbo)
		{
			map_param.set_position_vbo(nullptr);
			if(view->is_linked_to_map(map)) views_to_update.insert(view);
		}
		if(map_param.get_vector_vbo_index(vbo) >= 0)
		{
			map_param.remove_vector_vbo(vbo);
			if(view->is_linked_to_map(map)) views_to_update.insert(view);
		}
	}

	foreach(View* v, views_to_update)
		v->update();
}

void Plugin_SurfaceRenderVector::bb_changed()
{

}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
