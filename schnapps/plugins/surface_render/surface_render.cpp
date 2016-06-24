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
#include <schnapps/core/camera.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

MapParameters& Plugin_SurfaceRender::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	return parameter_set_[view][map];
}

bool Plugin_SurfaceRender::enable()
{
	dock_tab_ = new SurfaceRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render");

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

void Plugin_SurfaceRender::disable()
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

void Plugin_SurfaceRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
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
				switch (p.face_style_)
				{
					case MapParameters::FaceShadingStyle::FLAT:
						p.shader_flat_color_param_->bind(proj, mv);
						map->draw(cgogn::rendering::TRIANGLES);
						p.shader_flat_color_param_->release();
						break;
					case MapParameters::FaceShadingStyle::PHONG:
						if (p.get_normal_vbo())
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
						if (p.get_normal_vbo())
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

	if (p.render_boundary_)
	{
		if (p.get_position_vbo())
		{
			p.shader_simple_color_param_boundary_->bind(proj, mv);
			map->draw(cgogn::rendering::BOUNDARY);
			p.shader_simple_color_param_boundary_->release();
		}
	}
}

void Plugin_SurfaceRender::selected_view_changed(View* old, View* cur)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	const MapParameters& p = get_parameters(cur, map);
	dock_tab_->update_map_parameters(map, p);
}

void Plugin_SurfaceRender::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	View* view = schnapps_->get_selected_view();
	const MapParameters& p = get_parameters(view, cur);
	dock_tab_->update_map_parameters(cur, p);
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
}

void Plugin_SurfaceRender::schnapps_closing()
{

}

void Plugin_SurfaceRender::vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->add_normal_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->add_color_vbo(QString::fromStdString(vbo->name()));
		}
	}
}

void Plugin_SurfaceRender::vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map())
	{
		if (vbo->vector_dimension() == 3)
		{
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_normal_vbo(QString::fromStdString(vbo->name()));
			dock_tab_->remove_color_vbo(QString::fromStdString(vbo->name()));
		}
	}

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			if (p.get_position_vbo() == vbo)
				p.set_position_vbo(nullptr);
			if (p.get_normal_vbo() == vbo)
				p.set_normal_vbo(nullptr);
			if (p.get_color_vbo() == vbo)
				p.set_color_vbo(nullptr);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_SurfaceRender::bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
		}
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
