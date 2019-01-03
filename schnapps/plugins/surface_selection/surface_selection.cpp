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

#include <schnapps/plugins/surface_selection/surface_selection.h>
#include <schnapps/plugins/surface_selection/surface_selection_dock_tab.h>

#include <schnapps/core/schnapps.h>

#include <cgogn/geometry/algos/picking.h>

namespace schnapps
{

namespace plugin_surface_selection
{

Plugin_SurfaceSelection::Plugin_SurfaceSelection()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceSelection::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_SurfaceSelection::parameters(View* view, CMap2Handler* mh)
{
	cgogn_message_assert(view, "Try to access parameters for null view");
	cgogn_message_assert(mh, "Try to access parameters for null map handler");

	view->makeCurrent();

	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) == 0)
	{
		MapParameters& p = view_param_set[mh];
		p.mh_ = mh;
		p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(mh->map()->nb_cells<CMap2::Edge>())));
		return p;
	}
	else
		return view_param_set[mh];
}

bool Plugin_SurfaceSelection::check_docktab_activation()
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

bool Plugin_SurfaceSelection::enable()
{
	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new SurfaceSelection_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface Selection");

	return true;
}

void Plugin_SurfaceSelection::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

void Plugin_SurfaceSelection::draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh && dock_tab_->selected_map() == mh)
	{
		view->makeCurrent();
		QOpenGLFunctions* ogl = QOpenGLContext::currentContext()->functions();

		const MapParameters& p = parameters(view, mh);

		if (p.position_.is_valid() && p.cells_set_)
		{
			switch (p.cells_set_->orbit())
			{
				case CMap2::Vertex::ORBIT:
					if (p.cells_set_->nb_cells() > 0)
					{
						p.shader_point_sprite_param_selected_vertices_->bind(proj, mv);
						ogl->glDrawArrays(GL_POINTS, 0, p.cells_set_->nb_cells());
						p.shader_point_sprite_param_selected_vertices_->release();
					}
					if (p.selecting_ && p.selecting_vertex_.is_valid())
					{
						switch (p.selection_method_)
						{
							case NormalAngle:
							case SingleCell:
								p.shader_point_sprite_param_selection_sphere_->size_ = p.vertex_base_size_ * p.vertex_scale_factor_ * 1.01f;
								break;
							case WithinSphere:
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
				case CMap2::Edge::ORBIT:
					if (p.cells_set_->nb_cells() > 0)
					{
						p.shader_bold_line_param_selected_edges_->bind(proj, mv);
						ogl->glDrawArrays(GL_LINES, 0, p.cells_set_->nb_cells() * 2);
						p.shader_bold_line_param_selected_edges_->release();
					}
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case NormalAngle:
							case SingleCell:
								if (p.selecting_edge_.is_valid())
								{
									p.shader_bold_line_param_selection_edge_->bind(proj, mv);
									ogl->glDrawArrays(GL_LINES, 0, 2);
									p.shader_bold_line_param_selection_edge_->release();
								}
								break;
							case WithinSphere:
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
				case CMap2::Face::ORBIT:
					if (p.cells_set_->nb_cells() > 0)
					{
						p.shader_flat_param_selected_faces_->bind(proj, mv);
						ogl->glDrawArrays(GL_TRIANGLES, 0, p.selected_faces_nb_indices_);
						p.shader_flat_param_selected_faces_->release();
					}
					if (p.selecting_)
					{
						switch (p.selection_method_)
						{
							case NormalAngle:
							case SingleCell:
								if (p.selecting_face_.is_valid())
								{
									ogl->glDisable(GL_DEPTH_TEST);
									p.shader_simple_color_param_selection_face_->bind(proj, mv);
									ogl->glDrawArrays(GL_TRIANGLES, 0, p.selecting_face_nb_indices_);
									p.shader_simple_color_param_selection_face_->release();
									ogl->glEnable(GL_DEPTH_TEST);
								}
								break;
							case WithinSphere:
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
				default:
					break;
			}
		}
	}
}

bool Plugin_SurfaceSelection::keyPress(View* view, QKeyEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		if (event->key() == Qt::Key_S)
		{
			MapParameters& p = parameters(view, mh);

			if (p.position_.is_valid() && p.cells_set_)
			{
				view->setMouseTracking(true);
				p.selecting_ = true;

				// generate a false mouse move to update drawing on S keypressed !
				QPoint point = view->mapFromGlobal(QCursor::pos());
				QMouseEvent me = QMouseEvent(QEvent::MouseMove, point, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
				mouseMove(view, &me);

				view->update();
			}
		}
	}
	return true;
}

bool Plugin_SurfaceSelection::keyRelease(View* view, QKeyEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		if (event->key() == Qt::Key_S)
		{
			view->setMouseTracking(false);

			MapParameters& p = parameters(view, mh);
			p.selecting_ = false;

			view->update();
		}
	}
	return true;
}

bool Plugin_SurfaceSelection::mousePress(View* view, QMouseEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		if (p.selecting_ && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton))
		{
			CMapCellsSetGen* csg = p.cells_set_;
			switch (csg->orbit())
			{
				case CMap2::Vertex::ORBIT: {
					switch (p.selection_method_)
					{
						case SingleCell:
							if (p.selecting_vertex_.is_valid())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_vertex_.dart);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_vertex_.dart);
								p.update_selected_cells_rendering();
							}
							break;
						case WithinSphere: {
							if (p.selecting_vertex_.is_valid())
							{
								cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*mh->map(), p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.position_);
								neighborhood.collect(p.selecting_vertex_);
								if (event->button() == Qt::LeftButton)
									csg->select(neighborhood.cells(CMap2::Vertex::ORBIT));
								else if (event->button() == Qt::RightButton)
									csg->unselect(neighborhood.cells(CMap2::Vertex::ORBIT));
							}
						}
							break;
						case NormalAngle:
							break;
					}
				}
					break;
				case CMap2::Edge::ORBIT: {
					switch (p.selection_method_)
					{
						case SingleCell:
							if (p.selecting_edge_.is_valid())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_edge_.dart);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_edge_.dart);
								p.update_selected_cells_rendering();
							}
							break;
						case WithinSphere: {
							if (p.selecting_vertex_.is_valid())
							{
								cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*mh->map(), p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.position_);
								neighborhood.collect(p.selecting_vertex_);
								if (event->button() == Qt::LeftButton)
									csg->select(neighborhood.cells(CMap2::Edge::ORBIT));
								else if (event->button() == Qt::RightButton)
									csg->unselect(neighborhood.cells(CMap2::Edge::ORBIT));
							}
						}
							break;
						case NormalAngle:
							break;
					}
				}
					break;
				case CMap2::Face::ORBIT: {
					switch (p.selection_method_)
					{
						case SingleCell:
							if (p.selecting_face_.is_valid())
							{
								if (event->button() == Qt::LeftButton)
									csg->select(p.selecting_face_.dart);
								else if (event->button() == Qt::RightButton)
									csg->unselect(p.selecting_face_.dart);
								p.update_selected_cells_rendering();
							}
							break;
						case WithinSphere: {
							if (p.selecting_vertex_.is_valid())
							{
								cgogn::geometry::Collector_WithinSphere<VEC3, CMap2> neighborhood(*mh->map(), p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_, p.position_);
								neighborhood.collect(p.selecting_vertex_);
								if (event->button() == Qt::LeftButton)
									csg->select(neighborhood.cells(CMap2::Face::ORBIT));
								else if (event->button() == Qt::RightButton)
									csg->unselect(neighborhood.cells(CMap2::Face::ORBIT));
							}
						}
							break;
						case NormalAngle:
							break;
					}
				}
					break;
				default:
					break;
			}
		}
	}
	return true;
}

bool Plugin_SurfaceSelection::mouseMove(View* view, QMouseEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		if (p.selecting_)
		{
			qoglviewer::Vec P = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 0.0), &mh->frame());
			qoglviewer::Vec Q = view->camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(), event->y(), 1.0), &mh->frame());

			// TODO: apply inverse local map transformation

			// put in VEC3 format
			VEC3 A(P.x, P.y, P.z);
			VEC3 B(Q.x, Q.y, Q.z);

			switch(p.cells_set_->orbit())
			{
				case CMap2::Vertex::ORBIT:
				{
					std::vector<CMap2::Vertex> picked;
					cgogn::geometry::picking(*mh->map(), p.position_, A, B, picked);
					if (!picked.empty())
					{
						if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !mh->map()->same_cell(picked[0], p.selecting_vertex_)))
						{
							p.selecting_vertex_ = picked[0];
							std::vector<VEC3> selection_point{ p.position_[p.selecting_vertex_] };
							cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_);
						}
					}
				}
					break;
				case CMap2::Edge::ORBIT:
				{
					switch (p.selection_method_)
					{
						case SingleCell: {
							std::vector<CMap2::Edge> picked;
							cgogn::geometry::picking(*mh->map(), p.position_, A, B, picked);
							if (!picked.empty())
							{
								if (!p.selecting_edge_.is_valid() || (p.selecting_edge_.is_valid() && !mh->map()->same_cell(picked[0], p.selecting_edge_)))
								{
									p.selecting_edge_ = picked[0];
									auto vertices = mh->map()->vertices(CMap2::Edge(p.selecting_edge_));
									std::vector<VEC3> selection_segment{
										p.position_[vertices.first],
										p.position_[vertices.second]
									};
									cgogn::rendering::update_vbo(selection_segment, p.selection_edge_vbo_);
								}
							}
						}
							break;
						case WithinSphere: {
							std::vector<CMap2::Vertex> picked;
							cgogn::geometry::picking(*mh->map(), p.position_, A, B, picked);
							if (!picked.empty())
							{
								if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !mh->map()->same_cell(picked[0], p.selecting_vertex_)))
								{
									p.selecting_vertex_ = picked[0];
									std::vector<VEC3> selection_point{ p.position_[p.selecting_vertex_] };
									cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_);
								}
							}
						}
							break;
						case NormalAngle:
							break;
					}
				}
					break;
				case CMap2::Face::ORBIT:
				{
					switch (p.selection_method_)
					{
						case SingleCell:
						{
							std::vector<CMap2::Face> picked;
							cgogn::geometry::picking(*mh->map(), p.position_, A, B, picked);
							if (!picked.empty())
							{
								if (!p.selecting_face_.is_valid() || (p.selecting_face_.is_valid() && !mh->map()->same_cell(picked[0], p.selecting_face_)))
								{
									p.selecting_face_ = picked[0];
									std::vector<uint32> ears;
									cgogn::geometry::append_ear_triangulation(*mh->map(), CMap2::Face(p.selecting_face_), p.position_, ears);
									VEC3 c = cgogn::geometry::centroid(*mh->map(), CMap2::Face(p.selecting_face_), p.position_);
									std::vector<VEC3> selection_polygon;
									for (uint32 i : ears)
										selection_polygon.push_back(p.position_[i] * 0.9 + c * 0.1);
									p.selecting_face_nb_indices_ = selection_polygon.size();
									cgogn::rendering::update_vbo(selection_polygon, p.selection_face_vbo_);
								}
							}
						}
							break;
						case WithinSphere:
						{
							std::vector<CMap2::Vertex> picked;
							cgogn::geometry::picking(*mh->map(), p.position_, A, B, picked);
							if (!picked.empty())
							{
								if (!p.selecting_vertex_.is_valid() || (p.selecting_vertex_.is_valid() && !mh->map()->same_cell(picked[0], p.selecting_vertex_)))
								{
									p.selecting_vertex_ = picked[0];
									std::vector<VEC3> selection_point{ p.position_[p.selecting_vertex_] };
									cgogn::rendering::update_vbo(selection_point, p.selection_sphere_vbo_);
								}
							}
						}
							break;
						case NormalAngle:
							break;
					}
				}
					break;
				default:
					break;
			}

			view->update();
		}
	}
	return true;
}

bool Plugin_SurfaceSelection::wheelEvent(View* view, QWheelEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		if (p.selecting_)
		{
			switch (p.selection_method_)
			{
				case SingleCell:
					break;
				case WithinSphere:
					if (event->delta() > 0)
						p.selection_radius_scale_factor_ *= 0.9f;
					else
						p.selection_radius_scale_factor_ *= 1.1f;
					view->update();
					// TODO
					dock_tab_->spin_angle_radius->setValue(p.vertex_base_size_ * 10.0f * p.selection_radius_scale_factor_);
					break;
				case NormalAngle:
					break;
			}
		}
	}
	return true;
}

void Plugin_SurfaceSelection::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_SurfaceSelection::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_SurfaceSelection::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_SurfaceSelection::add_linked_map(View* view, CMap2Handler* mh)
{
	set_position_attribute(view, mh, setting_auto_load_position_attribute_, true);

	connect(mh, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_cells_set_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()), Qt::UniqueConnection);
}

void Plugin_SurfaceSelection::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_SurfaceSelection::remove_linked_map(View* view, CMap2Handler* mh)
{
	auto& view_param_set = parameter_set_[view];
	if (view_param_set.count(mh) > 0ul)
		view_param_set.erase(mh);

	disconnect(mh, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(attribute_changed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_changed(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_cells_set_removed(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(bb_changed()), this, SLOT(linked_map_bb_changed()));
}

void Plugin_SurfaceSelection::linked_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (!p.position_.is_valid() && name == setting_auto_load_position_attribute_)
					set_position_attribute(it.first, mh, name, true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceSelection::linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_.is_valid() && QString::fromStdString(p.position_.name()) == name)
					p.update_selected_cells_rendering();
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceSelection::linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		for (auto& it : parameter_set_)
		{
			std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
			if (view_param_set.count(mh) > 0ul)
			{
				MapParameters& p = view_param_set[mh];
				if (p.position_.is_valid() && QString::fromStdString(p.position_.name()) == name)
					set_position_attribute(it.first, mh, "", true);
				if (p.normal_.is_valid() && QString::fromStdString(p.normal_.name()) == name)
					set_normal_attribute(it.first, mh, "", true);
			}
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceSelection::linked_map_cells_set_removed(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	for (auto& it : parameter_set_)
	{
		std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			CMapCellsSetGen* cs = p.cells_set();
			if (cs && cs->orbit() == orbit && cs->name() == name)
				set_cells_set(it.first, mh, nullptr, true);
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_SurfaceSelection::linked_map_bb_changed()
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());
	uint32 nbe = mh->map()->nb_cells<CMap2::Edge>();

	for (auto& it : parameter_set_)
	{
		std::map<CMap2Handler*, MapParameters>& view_param_set = it.second;
		if (view_param_set.count(mh) > 0ul)
		{
			MapParameters& p = view_param_set[mh];
			p.set_vertex_base_size(mh->bb_diagonal_size() / (2 * std::sqrt(nbe)));
		}
	}

	for (View* view : mh->linked_views())
		view->update();
}

void Plugin_SurfaceSelection::viewer_initialized()
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

void Plugin_SurfaceSelection::set_position_attribute(View* view, CMap2Handler* mh, const QString& name, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_position_attribute(name);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_position_attribute(name);
		view->update();
	}
}

void Plugin_SurfaceSelection::set_normal_attribute(View* view, CMap2Handler* mh, const QString& name, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_normal_attribute(name);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_normal_attribute(name);
		view->update();
	}
}

void Plugin_SurfaceSelection::set_cells_set(View* view, CMap2Handler* mh, CMapCellsSetGen* cs, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_cells_set(cs);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_cells_set(cs);
		view->update();
	}
}

void Plugin_SurfaceSelection::set_selection_method(View* view, CMap2Handler* mh, SelectionMethod m, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.selection_method_ = m;
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_selection_method(m);
		view->update();
	}
}

void Plugin_SurfaceSelection::set_vertex_scale_factor(View* view, CMap2Handler* mh, float32 sf, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_vertex_scale_factor(sf);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_vertex_scale_factor(sf);
		view->update();
	}
}

void Plugin_SurfaceSelection::set_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab)
{
	if (view && view->is_linked_to_plugin(this) && mh && mh->is_linked_to_view(view))
	{
		MapParameters& p = parameters(view, mh);
		p.set_color(color);
		if (update_dock_tab && view->is_selected_view() && dock_tab_->selected_map() == mh)
			dock_tab_->set_color(color);
		view->update();
	}
}

} // namespace plugin_surface_selection

} // namespace schnapps
