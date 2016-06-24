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

#include <selection.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/picking.h>

namespace schnapps
{

Plugin_Selection::Plugin_Selection()
{}

MapParameters& Plugin_Selection::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();
	MapParameters& p = parameter_set_[view][map];
	p.map_ = static_cast<MapHandler<CMap2>*>(map);
	return p;
}

bool Plugin_Selection::enable()
{
	dock_tab_ = cgogn::make_unique<Selection_DockTab>(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_.get(), "Selection");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));

	return true;
}

void Plugin_Selection::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_.get());

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

void Plugin_Selection::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->is_selected_map())
	{
		view->makeCurrent();
		const MapParameters& p = get_parameters(view, map);
		if (p.get_position_attribute().is_valid() && p.cells_set_)
		{
			CellType ct = static_cast<CellType>(dock_tab_->combo_cellType->currentIndex());
			switch (ct)
			{
				case Dart_Cell:
					break;
				case Vertex_Cell:
					if (p.selecting_ && p.selecting_vertex_.is_valid())
					{
						std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
						cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
						p.shader_point_sprite_param_->size_ = p.vertex_base_size_ * p.vertex_scale_factor_;
						p.shader_point_sprite_param_->bind(proj, mv);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						QOpenGLFunctions* ogl = QOpenGLContext::currentContext()->functions();
						ogl->glDrawArrays(GL_POINTS, 0, 1);
						glDisable(GL_BLEND);
						p.shader_point_sprite_param_->release();
					}
					break;
				case Edge_Cell:
					break;
				case Face_Cell:
					break;
				case Volume_Cell:
					break;
			}
		}
	}
}

void Plugin_Selection::keyPress(View* view, QKeyEvent* event)
{
	if (event->key() == Qt::Key_Shift)
	{
		view->setMouseTracking(true);

		MapHandlerGen* map = schnapps_->get_selected_map();
		MapParameters& p = get_parameters(view, map);
		p.selecting_ = true;

		// generate a false mouse move to update drawing on shift keypressed !
		QPoint point = schnapps_->get_selected_view()->mapFromGlobal(QCursor::pos());
		QMouseEvent me = QMouseEvent(QEvent::MouseMove, point, Qt::NoButton, Qt::NoButton, Qt::ShiftModifier);
		mouseMove(view, &me);

		view->update();
	}
}

void Plugin_Selection::keyRelease(View* view, QKeyEvent* event)
{
	if (event->key() == Qt::Key_Shift)
	{
		view->setMouseTracking(false);

		MapHandlerGen* map = schnapps_->get_selected_map();
		MapParameters& p = get_parameters(view, map);
		p.selecting_ = false;

		view->update();
	}
}

void Plugin_Selection::mousePress(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	MapParameters& p = get_parameters(view, map);
	if (p.selecting_ && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton))
	{
		if (p.get_position_attribute().is_valid() && p.cells_set_)
		{
			CellType ct = static_cast<CellType>(dock_tab_->combo_cellType->currentIndex());
			switch (ct)
			{
				case Dart_Cell:
					break;
				case Vertex_Cell:
					if (p.selecting_vertex_.is_valid())
					{
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
								if(event->button() == Qt::LeftButton)
									p.cells_set_->select(p.selecting_vertex_);
								else if(event->button() == Qt::RightButton)
									p.cells_set_->unselect(p.selecting_vertex_);
								break;
								break;
							case MapParameters::WithinSphere:
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
					break;
				case Edge_Cell:
					break;
				case Face_Cell:
					break;
				case Volume_Cell:
					break;
			}
		}
	}
}

void Plugin_Selection::mouseMove(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	MapParameters& p = get_parameters(view, map);
	if (p.selecting_)
	{
		if (p.get_position_attribute().is_valid() && p.cells_set_)
		{
			CellType ct = static_cast<CellType>(dock_tab_->combo_cellType->currentIndex());

			qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &map->get_frame());
			qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &map->get_frame());

//			// TODO: apply inverse local map transformation

			// put in VEC3 format
			VEC3 A(P.x, P.y, P.z);
			VEC3 B(Q.x, Q.y, Q.z);

			CMap2* map2 = static_cast<MapHandler<CMap2>*>(map)->get_map();

			switch(ct)
			{
				case Dart_Cell:
					break;
				case Vertex_Cell: {
					std::vector<MapHandler<CMap2>::Vertex> picked;
					if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
						p.selecting_vertex_ = picked[0];
				}
					break;
				case Edge_Cell:
					break;
				case Face_Cell:
					break;
				case Volume_Cell:
					break;
			}

			view->update();
		}
	}
}

void Plugin_Selection::wheelEvent(View* view, QWheelEvent* event)
{

}

void Plugin_Selection::selected_view_changed(View* old, View* cur)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	const MapParameters& p = get_parameters(cur, map);
	dock_tab_->update_map_parameters(map, p);
}

void Plugin_Selection::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (old)
	{
		disconnect(old, SIGNAL(cells_set_added(CellType, const QString&)), dock_tab_.get(), SLOT(selected_map_cells_set_added(CellType, const QString&)));
		disconnect(old, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), dock_tab_.get(), SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(old, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_changed(cgogn::Orbit, const QString&)));
		disconnect(old, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(old, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		disconnect(old, SIGNAL(bb_changed()), this, SLOT(selected_map_bb_changed()));
	}
	if (cur)
	{
		connect(cur, SIGNAL(cells_set_added(CellType, const QString&)), dock_tab_.get(), SLOT(selected_map_cells_set_added(CellType, const QString&)));
		connect(cur, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), dock_tab_.get(), SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(cur, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_changed(cgogn::Orbit, const QString&)));
		connect(cur, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		connect(cur, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		connect(cur, SIGNAL(bb_changed()), this, SLOT(selected_map_bb_changed()));

		View* view = schnapps_->get_selected_view();
		const MapParameters& p = get_parameters(view, cur);
		dock_tab_->update_map_parameters(cur, p);
	}
}

void Plugin_Selection::selected_map_attribute_changed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.get_position_attribute_name() == name)
				{
					p.update_selected_cells_rendering();
					it.first->update();
				}
			}
		}

		dock_tab_->selected_map_attribute_removed(orbit, name);

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_Selection::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.get_position_attribute_name() == name)
				{
					p.set_position_attribute("");
					p.update_selected_cells_rendering();
					it.first->update();
				}
				if (p.get_normal_attribute_name() == name)
					p.set_normal_attribute("");
			}
		}

		dock_tab_->selected_map_attribute_removed(orbit, name);
	}
}

void Plugin_Selection::selected_map_connectivity_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			if (p.get_position_attribute().is_valid())
				p.update_selected_cells_rendering();
		}
	}
}

void Plugin_Selection::selected_map_bb_changed()
{
	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
	MapParameters& p = get_parameters(view, map);
	p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
