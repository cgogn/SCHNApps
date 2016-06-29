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

#include <cgogn/geometry/algos/picking.h>
#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

Plugin_Selection::Plugin_Selection()
{}

MapParameters& Plugin_Selection::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.map_ = static_cast<MapHandler<CMap2>*>(map);
		p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_Selection::enable()
{
	dock_tab_ = cgogn::make_unique<Selection_DockTab>(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_.get(), "Selection");

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));

	update_dock_tab();

	return true;
}

void Plugin_Selection::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_.get());

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_dock_tab()));
}

void Plugin_Selection::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map->dimension() == 2 && map->is_selected_map())
	{
		view->makeCurrent();
		QOpenGLFunctions* ogl = QOpenGLContext::currentContext()->functions();

		const MapParameters& p = get_parameters(view, map);

		if (p.get_position_attribute().is_valid() && p.cells_set_)
		{
			switch (p.cells_set_->get_cell_type())
			{
				case Dart_Cell:
					break;
				case Vertex_Cell:
					if (p.cells_set_->get_nb_cells() > 0)
					{
						p.shader_point_sprite_param_selected_vertices_->bind(proj, mv);
						ogl->glDrawArrays(GL_POINTS, 0, p.cells_set_->get_nb_cells());
						p.shader_point_sprite_param_selected_vertices_->release();
					}
					if (p.selecting_ && p.selecting_vertex_.is_valid())
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
							case MapParameters::SingleCell:
								p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * p.vertex_scale_factor_;
								break;
							case MapParameters::WithinSphere:
								p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_;
								break;
						}
						p.shader_point_sprite_param_selection_sphere_->bind(proj, mv);
						ogl->glEnable(GL_BLEND);
						ogl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						ogl->glDrawArrays(GL_POINTS, 0, 1);
						ogl->glDisable(GL_BLEND);
						p.shader_point_sprite_param_selection_sphere_->release();
					}
					break;
				case Edge_Cell:
					if (p.cells_set_->get_nb_cells() > 0)
					{
						p.shader_bold_line_param_selected_edges_->bind(proj, mv);
						ogl->glDrawArrays(GL_LINES, 0, p.cells_set_->get_nb_cells() * 2);
						p.shader_bold_line_param_selected_edges_->release();
					}
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
							case MapParameters::SingleCell:
								if (p.selecting_edge_.is_valid())
								{
									p.shader_bold_line_param_selection_edge_->bind(proj, mv);
									ogl->glDrawArrays(GL_LINES, 0, 2);
									p.shader_bold_line_param_selection_edge_->release();
								}
								break;
							case MapParameters::WithinSphere:
								if (p.selecting_vertex_.is_valid())
								{
									p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_;
									p.shader_point_sprite_param_selection_sphere_->bind(proj, mv);
									ogl->glEnable(GL_BLEND);
									ogl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									ogl->glDrawArrays(GL_POINTS, 0, 1);
									ogl->glDisable(GL_BLEND);
									p.shader_point_sprite_param_selection_sphere_->release();
								}
								break;
						}
					}
					break;
				case Face_Cell:
					if (p.cells_set_->get_nb_cells() > 0)
					{
						p.shader_simple_color_param_selected_faces_->bind(proj, mv);
						ogl->glDrawArrays(GL_TRIANGLES, 0, p.selected_faces_nb_indices_);
						p.shader_simple_color_param_selected_faces_->release();
					}
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
							case MapParameters::SingleCell:
								if (p.selecting_face_.is_valid())
								{
									p.shader_simple_color_param_selection_face_->bind(proj, mv);
									ogl->glDrawArrays(GL_TRIANGLES, 0, p.selecting_face_nb_indices_);
									p.shader_simple_color_param_selection_face_->release();
								}
								break;
							case MapParameters::WithinSphere:
								if (p.selecting_vertex_.is_valid())
								{
									p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_;
									p.shader_point_sprite_param_selection_sphere_->bind(proj, mv);
									ogl->glEnable(GL_BLEND);
									ogl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									ogl->glDrawArrays(GL_POINTS, 0, 1);
									ogl->glDisable(GL_BLEND);
									p.shader_point_sprite_param_selection_sphere_->release();
								}
								break;
						}
					}
					break;
				case Volume_Cell:
					break;
			}
		}
	}
}

void Plugin_Selection::keyPress(View* view, QKeyEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		if (event->key() == Qt::Key_Shift)
		{
			view->setMouseTracking(true);

			MapParameters& p = get_parameters(view, map);
			p.selecting_ = true;

			// generate a false mouse move to update drawing on shift keypressed !
			QPoint point = view->mapFromGlobal(QCursor::pos());
			QMouseEvent me = QMouseEvent(QEvent::MouseMove, point, Qt::NoButton, Qt::NoButton, Qt::ShiftModifier);
			mouseMove(view, &me);

			view->update();
		}
	}
}

void Plugin_Selection::keyRelease(View* view, QKeyEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		if (event->key() == Qt::Key_Shift)
		{
			view->setMouseTracking(false);

			MapParameters& p = get_parameters(view, map);
			p.selecting_ = false;

			view->update();
		}
	}
}

void Plugin_Selection::mousePress(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		if (p.selecting_ && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton))
		{
			if (p.get_position_attribute().is_valid() && p.cells_set_)
			{
				switch (p.cells_set_->get_cell_type())
				{
					case Dart_Cell:
						break;
					case Vertex_Cell: {
						CellsSet<CMap2, MapHandler<CMap2>::Vertex>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Vertex>*>(p.cells_set_);
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
								if (p.selecting_vertex_.is_valid())
								{
									if(event->button() == Qt::LeftButton)
										tcs->select(p.selecting_vertex_);
									else if(event->button() == Qt::RightButton)
										tcs->unselect(p.selecting_vertex_);
									p.update_selected_cells_rendering();
								}
								break;
							case MapParameters::WithinSphere: {
								if (p.selecting_vertex_.is_valid())
								{
									CMap2* map2 = static_cast<MapHandler<CMap2>*>(map)->get_map();
									cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*map2, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());
									neighborhood.collect(p.selecting_vertex_);
									neighborhood.foreach_cell([&] (CMap2::Vertex v)
									{
										if(event->button() == Qt::LeftButton)
											tcs->select(v);
										else if(event->button() == Qt::RightButton)
											tcs->unselect(v);
									});
								}
							}
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
						break;
					case Edge_Cell: {
						CellsSet<CMap2, MapHandler<CMap2>::Edge>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Edge>*>(p.cells_set_);
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
								if (p.selecting_edge_.is_valid())
								{
									if(event->button() == Qt::LeftButton)
										tcs->select(p.selecting_edge_);
									else if(event->button() == Qt::RightButton)
										tcs->unselect(p.selecting_edge_);
									p.update_selected_cells_rendering();
								}
								break;
							case MapParameters::WithinSphere: {
								if (p.selecting_vertex_.is_valid())
								{
									CMap2* map2 = static_cast<MapHandler<CMap2>*>(map)->get_map();
									cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*map2, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());
									neighborhood.collect(p.selecting_vertex_);
									neighborhood.foreach_cell([&] (CMap2::Edge e)
									{
										if(event->button() == Qt::LeftButton)
											tcs->select(e);
										else if(event->button() == Qt::RightButton)
											tcs->unselect(e);
									});
								}
							}
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
						break;
					case Face_Cell: {
						CellsSet<CMap2, MapHandler<CMap2>::Face>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Face>*>(p.cells_set_);
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
								if (p.selecting_face_.is_valid())
								{
									if(event->button() == Qt::LeftButton)
										tcs->select(p.selecting_face_);
									else if(event->button() == Qt::RightButton)
										tcs->unselect(p.selecting_face_);
									p.update_selected_cells_rendering();
								}
								break;
							case MapParameters::WithinSphere: {
								if (p.selecting_vertex_.is_valid())
								{
									CMap2* map2 = static_cast<MapHandler<CMap2>*>(map)->get_map();
									cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*map2, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());
									neighborhood.collect(p.selecting_vertex_);
									neighborhood.foreach_cell([&] (CMap2::Face f)
									{
										if(event->button() == Qt::LeftButton)
											tcs->select(f);
										else if(event->button() == Qt::RightButton)
											tcs->unselect(f);
									});
								}
							}
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
						break;
					case Volume_Cell:
						break;
				}
			}
		}
	}
}

void Plugin_Selection::mouseMove(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		if (p.selecting_)
		{
			if (p.get_position_attribute().is_valid() && p.cells_set_)
			{
				qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &map->get_frame());
				qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &map->get_frame());

	//			// TODO: apply inverse local map transformation

				// put in VEC3 format
				VEC3 A(P.x, P.y, P.z);
				VEC3 B(Q.x, Q.y, Q.z);

				CMap2* map2 = static_cast<MapHandler<CMap2>*>(map)->get_map();

				switch(p.cells_set_->get_cell_type())
				{
					case Dart_Cell:
						break;
					case Vertex_Cell:
					{
						std::vector<MapHandler<CMap2>::Vertex> picked;
						if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
						{
							if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !map2->same_cell(picked[0], p.selecting_vertex_)))
							{
								p.selecting_vertex_ = picked[0];
								std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
								cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
							}
						}
					}
						break;
					case Edge_Cell:
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
								break;
							case MapParameters::SingleCell: {
								std::vector<MapHandler<CMap2>::Edge> picked;
								if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
								{
									if (!p.selecting_edge_.is_valid() || (p.selecting_edge_.is_valid() && !map2->same_cell(picked[0], p.selecting_edge_)))
									{
										p.selecting_edge_ = picked[0];
										std::pair<MapHandler<CMap2>::Vertex, MapHandler<CMap2>::Vertex> vertices = map2->vertices(p.selecting_edge_);
										std::vector<VEC3> selection_segment{
											p.get_position_attribute()[vertices.first],
											p.get_position_attribute()[vertices.second]
										};
										cgogn::rendering::update_vbo(selection_segment, p.selection_edge_vbo_.get());
									}
								}
							}
								break;
							case MapParameters::WithinSphere: {
								std::vector<MapHandler<CMap2>::Vertex> picked;
								if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
								{
									if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !map2->same_cell(picked[0], p.selecting_vertex_)))
									{
										p.selecting_vertex_ = picked[0];
										std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
										cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
									}
								}
							}
								break;
						}
					}
						break;
					case Face_Cell:
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
								break;
							case MapParameters::SingleCell: {
								std::vector<MapHandler<CMap2>::Face> picked;
								if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
								{
									if (!p.selecting_face_.is_valid() || (p.selecting_face_.is_valid() && !map2->same_cell(picked[0], p.selecting_face_)))
									{
										p.selecting_face_ = picked[0];
										std::vector<VEC3> selection_polygon;
										std::vector<uint32> ears;
										cgogn::geometry::append_ear_triangulation<VEC3>(*map2, p.selecting_face_, p.get_position_attribute(), ears);
										for (uint32 i : ears)
											selection_polygon.push_back(p.get_position_attribute()[i]);
										p.selecting_face_nb_indices_ = selection_polygon.size();
										cgogn::rendering::update_vbo(selection_polygon, p.selection_face_vbo_.get());
									}
								}
							}
								break;
							case MapParameters::WithinSphere: {
								std::vector<MapHandler<CMap2>::Vertex> picked;
								if (cgogn::geometry::picking<VEC3>(*map2, p.get_position_attribute(), A, B, picked))
								{
									if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !map2->same_cell(picked[0], p.selecting_vertex_)))
									{
										p.selecting_vertex_ = picked[0];
										std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
										cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
									}
								}
							}
								break;
						}
					}
						break;
					case Volume_Cell:
						break;
				}

				view->update();
			}
		}
	}
}

void Plugin_Selection::wheelEvent(View* view, QWheelEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(view, map);
		if (p.selecting_)
		{
			switch (p.selection_method_)
			{
				case MapParameters::SingleCell:
					break;
				case MapParameters::WithinSphere:
					if (event->delta() > 0)
						p.selection_radius_scale_factor_ *= 0.9f;
					else
						p.selection_radius_scale_factor_ *= 1.1f;
					view->update();
					dock_tab_.get()->spin_angle_radius->setValue(p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_);
					break;
				case MapParameters::NormalAngle:
					break;
			}
		}
	}
}

void Plugin_Selection::view_linked(View* view)
{
	update_dock_tab();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_linked(map); }
}

void Plugin_Selection::view_unlinked(View* view)
{
	update_dock_tab();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { map_unlinked(map); }
}

void Plugin_Selection::map_linked(MapHandlerGen* map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		connect(map, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(linked_map_cells_set_added(CellType, const QString&)), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
		connect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
		connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
	}
}

void Plugin_Selection::map_unlinked(MapHandlerGen* map)
{
	update_dock_tab();

	if (map->dimension() == 2)
	{
		disconnect(map, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(linked_map_cells_set_added(CellType, const QString&)));
		disconnect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)));
		disconnect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
	}
}

void Plugin_Selection::linked_map_cells_set_added(CellType ct, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map())
		dock_tab_->selected_map_cells_set_added(ct, name);
}

void Plugin_Selection::linked_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map() && map->cell_type(orbit) == Vertex_Cell)
		dock_tab_->selected_map_vertex_attribute_added(name);
}

void Plugin_Selection::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name)
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
	}
}

void Plugin_Selection::linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());

	if (map->is_selected_map() && map->cell_type(orbit) == Vertex_Cell)
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

		dock_tab_->selected_map_vertex_attribute_removed(name);
	}
}

void Plugin_Selection::linked_map_bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
	uint32 nbe = map->nb_cells(Edge_Cell);

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(nbe)));
		}
	}
}

void Plugin_Selection::update_dock_tab()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();
	if (view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
	{
		schnapps_->enable_plugin_tab_widgets(this);
		const MapParameters& p = get_parameters(view, map);
		dock_tab_->update_map_parameters(map, p);
	}
	else
		schnapps_->disable_plugin_tab_widgets(this);
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
