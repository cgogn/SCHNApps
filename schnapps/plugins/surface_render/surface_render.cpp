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

namespace schnapps
{

MapParameters& Plugin_SurfaceRender::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	return parameter_set_[view][map];
}

bool Plugin_SurfaceRender::enable()
{
//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(m_schnapps->getStaticPointers());

	dock_tab_ = new SurfaceRender_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render");

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

void Plugin_SurfaceRender::disable()
{
	delete dock_tab_;
}

void Plugin_SurfaceRender::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
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

	if(map == schnapps_->get_selected_map())
	{
		if(vbo->vector_dimension() == 3)
		{
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->add_normal_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->add_color_vbo(QString::fromStdString(vbo->get_name()));
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
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->remove_normal_vbo(QString::fromStdString(vbo->get_name()));
			dock_tab_->remove_color_vbo(QString::fromStdString(vbo->get_name()));
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

	const QList<View*>& views = map->get_linked_views();
	foreach(View* view, views)
	{
		if (parameter_set_.contains(view))
		{
			MapParameters& p = get_parameters(view, map);
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges())));
		}
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
