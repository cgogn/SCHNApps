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

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

MapParameters& Plugin_VolumeRender::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	return parameter_set_[view][map];
}

bool Plugin_VolumeRender::enable()
{
	dock_tab_ = new VolumeRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Volume Render");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });

	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map)
	{
		View* view = schnapps_->get_selected_view();
		const MapParameters& p = get_parameters(view, map);
		dock_tab_->update_map_parameters(map, p);
	}

	return true;
}

void Plugin_VolumeRender::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_removed(map); });
}

void Plugin_VolumeRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	view->makeCurrent();
	const MapParameters& p = get_parameters(view, map);

	if (p.render_faces_)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		if (p.get_position_vbo())
		{
			if (p.get_color_vbo())
			{
				p.shader_expl_vol_col_->bind(proj, mv);
				map->draw(cgogn::rendering::TRIANGLES);
				p.shader_expl_vol_col_->release();
			}
			else
			{
				p.shader_expl_vol_->bind(proj, mv);
				map->draw(cgogn::rendering::TRIANGLES);
				p.shader_expl_vol_->release();
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
}

void Plugin_VolumeRender::selected_view_changed(View* old, View* cur)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	const MapParameters& p = get_parameters(cur, map);
	dock_tab_->update_map_parameters(map, p);
}

void Plugin_VolumeRender::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	View* view = schnapps_->get_selected_view();
	const MapParameters& p = get_parameters(view, cur);
	dock_tab_->update_map_parameters(cur, p);
}

void Plugin_VolumeRender::map_added(MapHandlerGen *map)
{
	connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_VolumeRender::map_removed(MapHandlerGen *map)
{
	disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_VolumeRender::schnapps_closing()
{

}

void Plugin_VolumeRender::vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map == schnapps_->get_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->add_color_vbo(QString::fromStdString(vbo->name()));
		}
	}
}

void Plugin_VolumeRender::vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map == schnapps_->get_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_color_vbo(QString::fromStdString(vbo->name()));
		}
	}

	std::set<View*> views_to_update;

	for (auto& it : parameter_set_)
	{
		View* view = it.first;
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		MapParameters& map_param = view_param_set[map];
		if (map_param.get_position_vbo() == vbo)
		{
			map_param.set_position_vbo(nullptr);
			if (view->is_linked_to_map(map)) views_to_update.insert(view);
		}
	}

	for (View* v : views_to_update)
		v->update();
}

void Plugin_VolumeRender::bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	for (View* view : map->get_linked_views())
	{
		if (parameter_set_.count(view) > 0ul)
		{
			MapParameters& p = get_parameters(view, map);
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(CellType::Edge_Cell))));
		}
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
