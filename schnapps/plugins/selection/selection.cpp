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

namespace schnapps
{

namespace plugin_selection
{

MapParameters& Plugin_Selection::get_parameters(View* view, MapHandlerGen* map)
{
	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(map) == 0)
	{
		MapParameters& p = view_param_set[map];
		p.map_ = map;
		p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
		return p;
	}
	else
		return view_param_set[map];
}

bool Plugin_Selection::check_docktab_activation()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
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

bool Plugin_Selection::enable()
{
	if (get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new Selection_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Selection");

	return true;
}

void Plugin_Selection::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

void Plugin_Selection::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	if (map && view && map->is_selected_map())
	{
		view->makeCurrent();
		QOpenGLFunctions* ogl = QOpenGLContext::currentContext()->functions();
		QOpenGLFunctions_3_3_Core* ogl33 = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

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
					if (p.selecting_ && !p.selecting_vertex_.is_nil())
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
							case MapParameters::SingleCell:
								p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * p.vertex_scale_factor_ * 1.01f;
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
								if (!p.selecting_edge_.is_nil())
								{
									p.shader_bold_line_param_selection_edge_->bind(proj, mv);
									ogl->glDrawArrays(GL_LINES, 0, 2);
									p.shader_bold_line_param_selection_edge_->release();
								}
								break;
							case MapParameters::WithinSphere:
								if (!p.selecting_vertex_.is_nil())
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
						p.shader_flat_param_selected_faces_->bind(proj, mv);
						ogl->glDrawArrays(GL_TRIANGLES, 0, p.selected_faces_nb_indices_);
						p.shader_flat_param_selected_faces_->release();
					}
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case MapParameters::NormalAngle:
							case MapParameters::SingleCell:
								if (!p.selecting_face_.is_nil())
								{
									ogl->glDisable(GL_DEPTH_TEST);
									p.shader_simple_color_param_selection_face_->bind(proj, mv);
									ogl->glDrawArrays(GL_TRIANGLES, 0, p.selecting_face_nb_indices_);
									p.shader_simple_color_param_selection_face_->release();
									ogl->glEnable(GL_DEPTH_TEST);
								}
								break;
							case MapParameters::WithinSphere:
								if (!p.selecting_vertex_.is_nil())
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
					if (p.cells_set_->get_nb_cells() > 0)
						p.drawer_rend_selected_volumes_->draw(proj, mv, ogl33);
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
								if (!p.selecting_volume_.is_nil())
									p.drawer_rend_selected_volumes_->draw(proj, mv, ogl33);
								break;
							default:
								break;
						}
					}
					break;
				default:
					break;
			}
		}
	}
}

void Plugin_Selection::keyPress(View* view, QKeyEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view))
	{
		if (event->key() == Qt::Key_Shift)
		{
			MapParameters& p = get_parameters(view, map);

			if (p.get_position_attribute().is_valid() && p.cells_set_)
			{
				view->setMouseTracking(true);
				p.selecting_ = true;

				// generate a false mouse move to update drawing on shift keypressed !
				QPoint point = view->mapFromGlobal(QCursor::pos());
				QMouseEvent me = QMouseEvent(QEvent::MouseMove, point, Qt::NoButton, Qt::NoButton, Qt::ShiftModifier);
				mouseMove(view, &me);

				view->update();
			}
		}
	}
}

void Plugin_Selection::keyRelease(View* view, QKeyEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view))
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

bool Plugin_Selection::mousePress(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		if (p.selecting_ && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton))
		{
			CellsSetGen* csg = p.cells_set_;
			switch (csg->get_cell_type())
			{
				case Dart_Cell:
					break;
				case Vertex_Cell: {
					switch (p.selection_method_)
					{
						case MapParameters::SingleCell:
							if (!p.selecting_vertex_.is_nil())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_vertex_);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_vertex_);
								p.update_selected_cells_rendering();
							}
							break;
						case MapParameters::WithinSphere: {
							if (!p.selecting_vertex_.is_nil())
							{
								auto neighborhood = collector_within_sphere(map, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());
								neighborhood->collect(p.selecting_vertex_);
								if (event->button() == Qt::LeftButton)
									csg->select(neighborhood->cells(map->orbit(CellType::Vertex_Cell)));
								else if (event->button() == Qt::RightButton)
									csg->unselect(neighborhood->cells(map->orbit(CellType::Vertex_Cell)));
							}
						}
							break;
						case MapParameters::NormalAngle:
							break;
					}
				}
					break;
				case Edge_Cell: {
					switch (p.selection_method_)
					{
						case MapParameters::SingleCell:
							if (!p.selecting_edge_.is_nil())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_edge_);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_edge_);
								p.update_selected_cells_rendering();
							}
							break;
						case MapParameters::WithinSphere: {
							if (!p.selecting_vertex_.is_nil())
							{
								auto neighborhood = collector_within_sphere(map, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());
								if (neighborhood)
								{
									neighborhood->collect(p.selecting_vertex_);
									const auto& cells = neighborhood->cells(map->orbit(CellType::Edge_Cell));
									for (auto e : cells)
									{
										if (event->button() == Qt::LeftButton)
											csg->select(e);
										else if (event->button() == Qt::RightButton)
											csg->unselect(e);
									}
								}
							}
						}
							break;
						case MapParameters::NormalAngle:
							break;
					}
				}
					break;
				case Face_Cell:
				{
					switch (p.selection_method_)
					{
						case MapParameters::SingleCell:
							if (!p.selecting_face_.is_nil())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_face_);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_face_);
								p.update_selected_cells_rendering();
							}
							break;
						case MapParameters::WithinSphere: {
							if (!p.selecting_vertex_.is_nil())
							{
								auto neighborhood = collector_within_sphere(map, p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.get_position_attribute());

								if (neighborhood)
								{
									neighborhood->collect(p.selecting_vertex_);
									const auto& cells = neighborhood->cells(map->orbit(CellType::Face_Cell));
									for (auto f : cells)
									{
										if (event->button() == Qt::LeftButton)
											csg->select(f);
										else if  (event->button() == Qt::RightButton)
											csg->unselect(f);
									}
								}
							}
						}
							break;
						case MapParameters::NormalAngle:
							break;
					}
				}
					break;
				case Volume_Cell:
				{
					switch (p.selection_method_)
					{
						case MapParameters::SingleCell:
							if (!p.selecting_volume_.is_nil())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_volume_);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_volume_);
								p.update_selected_cells_rendering();
							}
							break;
						case MapParameters::WithinSphere:
							break;
						case MapParameters::NormalAngle:
							break;
					}
				}
					break;
				default:
					break;
			}

			if (event->button() == Qt::RightButton)
				return false;
		}
	}
	return true;
}

void Plugin_Selection::mouseMove(View* view, QMouseEvent* event)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		if (p.selecting_)
		{
			if (p.get_position_attribute().is_valid() && p.cells_set_)
			{
				qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &map->get_frame());
				qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &map->get_frame());

				// TODO: apply inverse local map transformation

				// put in VEC3 format
				VEC3 A(P.x, P.y, P.z);
				VEC3 B(Q.x, Q.y, Q.z);

				CMap2* map2 = map->dimension() == 2 ? static_cast<CMap2Handler*>(map)->get_map() : nullptr;
				CMap3* map3 = map->dimension() == 3 ? static_cast<CMap3Handler*>(map)->get_map() : nullptr;

				switch(p.cells_set_->get_cell_type())
				{
					case Dart_Cell:
						break;
					case Vertex_Cell:
					{
						auto picked = get_picked_cells(map, CellType::Vertex_Cell, p.get_position_attribute(), A, B);
						if (!picked.empty())
						{
							if (p.selecting_vertex_.is_nil() || (!p.selecting_vertex_.is_nil() && !map->same_cell(picked[0],p.selecting_vertex_, CellType::Vertex_Cell)))
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
							case MapParameters::SingleCell: {
								auto picked = get_picked_cells(map, CellType::Edge_Cell, p.get_position_attribute(), A, B);
								if (!picked.empty())
								{
									if (p.selecting_edge_.is_nil() || (!p.selecting_edge_.is_nil() && !map->same_cell(picked[0],p.selecting_edge_, CellType::Edge_Cell)))
									{
										p.selecting_edge_ = picked[0];
										auto vertices = map->vertices((p.selecting_edge_));
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
								auto picked = get_picked_cells(map, CellType::Vertex_Cell, p.get_position_attribute(), A, B);
								if (!picked.empty())
								{
									if (p.selecting_vertex_.is_nil() || (!p.selecting_vertex_.is_nil() && !map->same_cell(picked[0],p.selecting_vertex_, CellType::Vertex_Cell)))
									{
										p.selecting_vertex_ = picked[0];
										std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
										cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
									}
								}
							}
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
						break;
					case Face_Cell:
					{
						switch (p.selection_method_)
						{
							case MapParameters::SingleCell:
							{
								auto picked = get_picked_cells(map, CellType::Face_Cell, p.get_position_attribute(), A, B);
								if (!picked.empty())
								{
									if (p.selecting_face_.is_nil() || (!p.selecting_face_.is_nil() && !map->same_cell(picked[0],p.selecting_face_, CellType::Face_Cell)))
									{
										p.selecting_face_ = picked[0];
										std::vector<VEC3> selection_polygon;
										std::vector<uint32> ears;
										VEC3 c;
										if (map->dimension() == 2)
										{
											const CMap2Handler::VertexAttribute<VEC3>& pos = static_cast<const CMap2Handler::VertexAttribute<VEC3>&>(p.get_position_attribute());
											cgogn::geometry::append_ear_triangulation<VEC3>(*map2, Face2(p.selecting_face_), pos, ears);
											c = cgogn::geometry::centroid<VEC3>(*map2, Face2(p.selecting_face_), pos);
										}
										else
										{
											const CMap3Handler::VertexAttribute<VEC3>& pos = static_cast<const CMap3Handler::VertexAttribute<VEC3>&>(p.get_position_attribute());
											cgogn::geometry::append_ear_triangulation<VEC3>(*map3, Face3(p.selecting_face_), pos, ears);
											c = cgogn::geometry::centroid<VEC3>(*map3, Face3(p.selecting_face_), pos);
										}

										for (uint32 i : ears)
											selection_polygon.push_back(p.get_position_attribute()[i] * 0.9 + c * 0.1);
										p.selecting_face_nb_indices_ = selection_polygon.size();
										cgogn::rendering::update_vbo(selection_polygon, p.selection_face_vbo_.get());
									}
								}
							}
								break;
							case MapParameters::WithinSphere:
							{
								auto picked = get_picked_cells(map, CellType::Vertex_Cell, p.get_position_attribute(), A, B);
								if (!picked.empty())
								{
									if (p.selecting_vertex_.is_nil() || (!p.selecting_vertex_.is_nil() && !map->same_cell(picked[0],p.selecting_vertex_, CellType::Vertex_Cell)))
									{
										p.selecting_vertex_ = picked[0];
										std::vector<VEC3> selection_point{p.get_position_attribute()[p.selecting_vertex_]};
										cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_.get());
									}
								}
							}
								break;
							case MapParameters::NormalAngle:
								break;
						}
					}
						break;
					case Volume_Cell:
						switch (p.selection_method_) {
							case MapParameters::SingleCell:
							{
								auto picked = get_picked_cells(map, CellType::Volume_Cell, p.get_position_attribute(), A, B);
								if (!picked.empty())
								{
									if (p.selecting_volume_.is_nil() || (!p.selecting_volume_.is_nil() && !map->same_cell(picked[0],p.selecting_volume_, CellType::Volume_Cell)))
										p.selecting_volume_ = picked[0];
								}
								break;
							}
							case MapParameters::WithinSphere:
								break;
							case MapParameters::NormalAngle:
								break;
						}
						break;
					default:
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
	if (map && map->is_linked_to_view(view))
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
					// TODO
					dock_tab_->spin_angle_radius->setValue(p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_);
					break;
				case MapParameters::NormalAngle:
					break;
			}
		}
	}
}

void Plugin_Selection::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { add_linked_map(view, map); }
}

void Plugin_Selection::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (MapHandlerGen* map : view->get_linked_maps()) { remove_linked_map(view, map); }
}

std::unique_ptr<Plugin_Selection::CollectorGen> Plugin_Selection::collector_within_sphere(MapHandlerGen* map, float64 radius, const MapHandlerGen::Attribute_T<VEC3>& position_att)
{
	if (map)
	{
		if (map->dimension() == 2)
		{
			CMap2Handler* mh2 = static_cast<CMap2Handler*>(map);
			const CMap2::VertexAttribute<VEC3>* pos = static_cast<const CMap2::VertexAttribute<VEC3>*>(&position_att);
			return cgogn::make_unique<cgogn::geometry::Collector_WithinSphere<VEC3, CMap2>>(*mh2->get_map(), radius, *pos);
		}
		else
		{
			CMap3Handler* mh3 = static_cast<CMap3Handler*>(map);
			const CMap3::VertexAttribute<VEC3>* pos = static_cast<const CMap3::VertexAttribute<VEC3>*>(&position_att);
			return cgogn::make_unique<cgogn::geometry::Collector_WithinSphere<VEC3, CMap3>>(*mh3->get_map(), radius, *pos);
		}
	}
	return nullptr;
}

std::vector<cgogn::Dart> Plugin_Selection::get_picked_cells(MapHandlerGen* map, CellType ct, const MapHandlerGen::Attribute_T<VEC3>& position_att, VEC3& A, VEC3& B)
{
	std::vector<cgogn::Dart> res;
	bool pick_res = false;
	if (map)
	{
		if (map->dimension() == 2)
		{
			CMap2Handler* mh2 = static_cast<CMap2Handler*>(map);
			const CMap2::VertexAttribute<VEC3>* pos = static_cast<const CMap2::VertexAttribute<VEC3>*>(&position_att);
			switch (ct) {
				case CellType::Vertex_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh2->get_map(), *pos, A, B, reinterpret_cast<std::vector<Vertex2>&>(res)); break;
				case CellType::Edge_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh2->get_map(), *pos, A, B, reinterpret_cast<std::vector<Edge2>&>(res)); break;
				case CellType::Face_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh2->get_map(), *pos, A, B, reinterpret_cast<std::vector<Face2>&>(res)); break;
				case CellType::Volume_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh2->get_map(), *pos, A, B, reinterpret_cast<std::vector<Volume>&>(res)); break;
				default:
					pick_res = false; break;
			}
		}
		else
		{
			CMap3Handler* mh3 = static_cast<CMap3Handler*>(map);
			const CMap3::VertexAttribute<VEC3>* pos = static_cast<const CMap3::VertexAttribute<VEC3>*>(&position_att);
			switch (ct) {
				case CellType::Vertex_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh3->get_map(), *pos, A, B, reinterpret_cast<std::vector<Vertex3>&>(res)); break;
				case CellType::Edge_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh3->get_map(), *pos, A, B, reinterpret_cast<std::vector<Edge3>&>(res)); break;
				case CellType::Face_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh3->get_map(), *pos, A, B, reinterpret_cast<std::vector<Face3>&>(res)); break;
				case CellType::Volume_Cell:
					pick_res = cgogn::geometry::picking<VEC3>(*mh3->get_map(), *pos, A, B, reinterpret_cast<std::vector<Volume>&>(res)); break;
				default:
					pick_res = false; break;
			}
		}
	}

	if (!pick_res)
		return std::vector<cgogn::Dart>();
	else
		return res;
}

void Plugin_Selection::map_linked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	add_linked_map(view, map);
}

void Plugin_Selection::add_linked_map(View* view, MapHandlerGen* map)
{
	MapParameters& p = get_parameters(view, map);
	p.set_position_attribute(setting_auto_load_position_attribute_);

	connect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
	connect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(map, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(linked_map_cells_set_removed(CellType, const QString&)), Qt::UniqueConnection);
	connect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_Selection::map_unlinked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	remove_linked_map(view, map);
}

void Plugin_Selection::remove_linked_map(View* view, MapHandlerGen* map)
{
	disconnect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(linked_map_cells_set_removed(CellType, const QString&)));
	disconnect(map, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_Selection::linked_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (!p.get_position_attribute().is_valid() && name == setting_auto_load_position_attribute_)
					set_position_attribute(it.first, map, name, true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_Selection::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.get_position_attribute_name() == name)
					p.update_selected_cells_rendering();
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_Selection::linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(map) > 0ul)
			{
				MapParameters& p = view_param_set[map];
				if (p.get_position_attribute_name() == name)
					set_position_attribute(it.first, map, "", true);
				if (p.get_normal_attribute_name() == name)
					set_normal_attribute(it.first, map, "", true);
			}
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_Selection::linked_map_cells_set_removed(CellType ct, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	for (auto& it : parameter_set_)
	{
		std::map<MapHandlerGen*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(map) > 0ul)
		{
			MapParameters& p = view_param_set[map];
			CellsSetGen* cs = p.get_cells_set();
			if (cs && cs->get_cell_type() == ct && cs->get_name() == name)
				set_cells_set(it.first, map, nullptr, true);
		}
	}

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_Selection::linked_map_bb_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());
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

	for (View* view : map->get_linked_views())
		view->update();
}

void Plugin_Selection::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	if (view && (this->parameter_set_.count(view) > 0))
	{
		auto& view_param_set = parameter_set_[view];
		for (auto & p : view_param_set)
			p.second.initialize_gl();
	}
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_Selection::set_position_attribute(View* view, MapHandlerGen* map, const QString& name, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.set_position_attribute(name);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_position_attribute(name);
		view->update();
	}
}

void Plugin_Selection::set_normal_attribute(View* view, MapHandlerGen* map, const QString& name, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.set_normal_attribute(name);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_normal_attribute(name);
		view->update();
	}
}

void Plugin_Selection::set_cells_set(View* view, MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.set_cells_set(cs);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_cells_set(cs);
		view->update();
	}
}

void Plugin_Selection::set_selection_method(View* view, MapHandlerGen* map, MapParameters::SelectionMethod m, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.selection_method_ = m;
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_selection_method(m);
		view->update();
	}
}

void Plugin_Selection::set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.set_vertex_scale_factor(sf);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_vertex_scale_factor(sf);
		view->update();
	}
}

void Plugin_Selection::set_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view))
	{
		MapParameters& p = get_parameters(view, map);
		p.set_color(color);
		if (update_dock_tab && view->is_selected_view() && map->is_selected_map())
			dock_tab_->set_color(color);
		view->update();
	}
}

} // namespace plugin_selection

} // namespace schnapps
