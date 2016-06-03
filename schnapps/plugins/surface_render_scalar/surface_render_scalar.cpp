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

#include <surface_render_scalar.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/geometry/algos/area.h>
#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

MapParameters& Plugin_SurfaceRenderScalar::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	MapParameters& p = parameter_set_[view][map];
	p.map_ = static_cast<MapHandler<CMap2>*>(map);
	return p;
}

bool Plugin_SurfaceRenderScalar::enable()
{
	dock_tab_ = new SurfaceRenderScalar_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Render Scalar");

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

void Plugin_SurfaceRenderScalar::disable()
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

void Plugin_SurfaceRenderScalar::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	view->makeCurrent();
	const MapParameters& p = get_parameters(view, map);

	if (p.get_position_vbo() && p.get_scalar_vbo())
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		p.shader_scalar_per_vertex_param_->bind(proj, mv);
		map->draw(cgogn::rendering::TRIANGLES);
		p.shader_scalar_per_vertex_param_->release();
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

void Plugin_SurfaceRenderScalar::selected_view_changed(View* old, View* cur)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	const MapParameters& p = get_parameters(cur, map);
	dock_tab_->update_map_parameters(map, p);
}

void Plugin_SurfaceRenderScalar::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	View* view = schnapps_->get_selected_view();
	const MapParameters& p = get_parameters(view, cur);
	dock_tab_->update_map_parameters(cur, p);
}

void Plugin_SurfaceRenderScalar::map_added(MapHandlerGen *map)
{
	connect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	connect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_SurfaceRenderScalar::map_removed(MapHandlerGen *map)
{
	disconnect(map, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(vbo_added(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(vbo_removed(cgogn::rendering::VBO*)));
	disconnect(map, SIGNAL(bb_changed()), this, SLOT(bb_changed()));
}

void Plugin_SurfaceRenderScalar::schnapps_closing()
{

}

void Plugin_SurfaceRenderScalar::vbo_added(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map == schnapps_->get_selected_map())
	{
		if (vbo->vector_dimension() == 3)
			dock_tab_->add_position_vbo(QString::fromStdString(vbo->name()));
		if (vbo->vector_dimension() == 1)
			dock_tab_->add_scalar_vbo(QString::fromStdString(vbo->name()));
	}
}

void Plugin_SurfaceRenderScalar::vbo_removed(cgogn::rendering::VBO* vbo)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map == schnapps_->get_selected_map())
	{
		if (vbo->vector_dimension() == 3)
			dock_tab_->remove_position_vbo(QString::fromStdString(vbo->name()));
		if (vbo->vector_dimension() == 1)
			dock_tab_->remove_scalar_vbo(QString::fromStdString(vbo->name()));
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
		if (map_param.get_scalar_vbo() == vbo)
		{
			map_param.set_scalar_vbo(nullptr);
			if (view->is_linked_to_map(map)) views_to_update.insert(view);
		}
	}

	for (View* v : views_to_update)
		v->update();
}

void Plugin_SurfaceRenderScalar::bb_changed()
{

}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
