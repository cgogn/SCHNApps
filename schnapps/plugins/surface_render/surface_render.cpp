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

#include <surface_render.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

bool Plugin_SurfaceRender::enable()
{
//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(m_schnapps->getStaticPointers());

	dock_tab_ = new SurfaceRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "SurfaceRender");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	foreach(MapHandlerGen* map, schnapps_->get_map_set().values())
		map_added(map);

	dock_tab_->updateMapParameters();

	return true;
}

void Plugin_SurfaceRender::disable()
{
	delete dock_tab_;
}

void Plugin_SurfaceRender::draw_map(View *view, MapHandlerGen *map)
{
}

void Plugin_SurfaceRender::selected_view_changed(View*, View*)
{
	dock_tab_->updateMapParameters();
}

void Plugin_SurfaceRender::selected_map_changed(MapHandlerGen*, MapHandlerGen*)
{
	dock_tab_->updateMapParameters();
}

void Plugin_SurfaceRender::map_added(MapHandlerGen *map)
{
	connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_SurfaceRender::map_removed(MapHandlerGen *map)
{
	disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));

	if(map == schnapps_->get_selected_map())
		dock_tab_->updateMapParameters();
}

void Plugin_SurfaceRender::schnapps_closing()
{

}

void Plugin_SurfaceRender::vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == schnapps_->get_selected_map())
	{
		if(vbo->vector_dimension() == 3)
		{
			dock_tab_->addPositionVBO(QString::fromStdString(vbo->get_name()));
			dock_tab_->addNormalVBO(QString::fromStdString(vbo->get_name()));
			dock_tab_->addColorVBO(QString::fromStdString(vbo->get_name()));
		}
	}
}

void Plugin_SurfaceRender::vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if(map == schnapps_->get_selected_map())
	{
		if(vbo->vector_dimension() == 3)
		{
			dock_tab_->removePositionVBO(QString::fromStdString(vbo->get_name()));
			dock_tab_->removeNormalVBO(QString::fromStdString(vbo->get_name()));
			dock_tab_->removeColorVBO(QString::fromStdString(vbo->get_name()));
		}
	}

	QSet<View*> views_to_update;

	for (auto i = parameter_set_.begin(); i != parameter_set_.end(); ++i)
	{
		View* view = i.key();
		QHash<MapHandlerGen*, MapParameters>& view_param_set = i.value();
		MapParameters& map_param = view_param_set[map];
		if(map_param.get_position_vbo() == vbo)
		{
			map_param.set_position_vbo(nullptr);
			if(view->is_linked_to_map(map)) views_to_update.insert(view);
		}
		if(map_param.get_normal_vbo() == vbo)
		{
			map_param.set_normal_vbo(nullptr);
			if(view->is_linked_to_map(map)) views_to_update.insert(view);
		}
		if(map_param.get_color_vbo() == vbo)
		{
			map_param.set_color_vbo(nullptr);
			if(view->is_linked_to_map(map)) views_to_update.insert(view);
		}
	}

	foreach(View* v, views_to_update)
		v->update();
}

void Plugin_SurfaceRender::bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	QList<View*> views = map->get_linked_views();
	foreach(View* v, views)
	{
		if (parameter_set_.contains(v))
			parameter_set_[v][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
