/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#include <schnapps/plugins/surface_deformation/surface_deformation.h>
#include <schnapps/plugins/surface_deformation/surface_deformation_dock_tab.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

Plugin_SurfaceDeformation::Plugin_SurfaceDeformation() :
	drag_init_(false),
	dragging_(false)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceDeformation::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_SurfaceDeformation::parameters(CMap2Handler* mh)
{
	cgogn_message_assert(mh, "Try to access parameters for null map");

	if (parameter_set_.count(mh) == 0)
	{
		MapParameters& p = parameter_set_[mh];
		p.mh_ = mh;
		p.working_cells_ = cgogn::make_unique<CMap2::CellCache>(*p.mh_->map());
		return p;
	}
	else
		return parameter_set_[mh];
}

bool Plugin_SurfaceDeformation::check_docktab_activation()
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

bool Plugin_SurfaceDeformation::enable()
{
	if (setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = add_setting("Auto load position attribute", "position").toString();

	dock_tab_ = new SurfaceDeformation_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Surface deformation");

	return true;
}

void Plugin_SurfaceDeformation::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

bool Plugin_SurfaceDeformation::keyPress(View* view, QKeyEvent* event)
{
	CMap2Handler* mh = dock_tab_->selected_map();
	if (mh && mh->is_linked_to_view(view))
	{
		switch (event->key())
		{
			case Qt::Key_D: {
				const MapParameters& p = parameters(mh);
				if (!dragging_ && p.handle_vertex_set_ && p.handle_vertex_set_->nb_cells() > 0)
					start_dragging(view);
				else
					stop_dragging(view);
				break;
			}

			case Qt::Key_R : {
				const MapParameters& p = parameters(mh);
				if (p.initialized_)
				{
					view->current_camera()->disable_views_bb_fitting();
					as_rigid_as_possible(mh);
					mh->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(p.position_.name()));

					for (View* view : mh->linked_views())
						view->update();
					view->current_camera()->enable_views_bb_fitting();
				}
				break;
			}
		}
	}

	return true;
}

void Plugin_SurfaceDeformation::start_dragging(View* view)
{
	dragging_ = true;
	drag_init_ = false;
	view->setMouseTracking(true);
	view->current_camera()->disable_views_bb_fitting();
}

void Plugin_SurfaceDeformation::stop_dragging(View* view)
{
	dragging_ = false;
	drag_init_ = false;
	view->setMouseTracking(false);
	view->current_camera()->enable_views_bb_fitting();
}

bool Plugin_SurfaceDeformation::mouseMove(View* view, QMouseEvent* event)
{
	if (dragging_)
	{
		CMap2Handler* mh = dock_tab_->selected_map();
		if (mh)
		{
			MapParameters& p = parameters(mh);
			if (!drag_init_)
			{
				drag_z_ = 0;
				p.handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					const VEC3& pp = p.position_[v];
					qoglviewer::Vec q = view->camera()->projectedCoordinatesOf(qoglviewer::Vec(pp[0],pp[1],pp[2]));
					drag_z_ += q.z;
				});
				drag_z_ /= p.handle_vertex_set_->nb_cells();

				qoglviewer::Vec pp(event->x(), event->y(), drag_z_);
				drag_previous_ = view->camera()->unprojectedCoordinatesOf(pp);

				drag_init_ = true;
			}
			else
			{
				qoglviewer::Vec pp(event->x(), event->y(), drag_z_);
				qoglviewer::Vec qq = view->camera()->unprojectedCoordinatesOf(pp);

				qoglviewer::Vec vec = qq - drag_previous_;
				VEC3 t(vec.x, vec.y, vec.z);
				p.handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					p.position_[v] += t;
				});

				drag_previous_ = qq;

				if (p.initialized_)
				{
					if (as_rigid_as_possible(mh))
					{
						mh->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(p.position_.name()));
						for (View* view : mh->linked_views())
							view->update();
					}
					else
					{
						// undo handle displacement & stop dragging
						p.handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
						{
							p.position_[v] -= t;
						});
						stop_dragging(view);
					}
				}
				else
				{
					mh->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(p.position_.name()));
					for (View* view : mh->linked_views())
						view->update();
				}
			}
		}
	}

	return true;
}

void Plugin_SurfaceDeformation::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			add_linked_map(view, mh);
	}
}

void Plugin_SurfaceDeformation::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	disconnect(view, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));

	for (Object* o : view->linked_objects())
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			remove_linked_map(view, mh);
	}
}

void Plugin_SurfaceDeformation::object_linked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		add_linked_map(view, mh);
}

void Plugin_SurfaceDeformation::add_linked_map(View*, CMap2Handler* mh)
{
	set_position_attribute(mh, setting_auto_load_position_attribute_, true);

	connect(mh, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(mh, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_cells_set_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
}

void Plugin_SurfaceDeformation::object_unlinked(Object* o)
{
	View* view = static_cast<View*>(sender());
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		remove_linked_map(view, mh);
}

void Plugin_SurfaceDeformation::remove_linked_map(View*, CMap2Handler* mh)
{
	if (parameter_set_.count(mh) > 0ul)
		parameter_set_.erase(mh);

	disconnect(mh, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_cells_set_removed(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDeformation::linked_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		if (parameter_set_.count(mh) > 0ul)
		{
			MapParameters& p = parameter_set_[mh];
			if (!p.position_attribute().is_valid() && name == setting_auto_load_position_attribute_)
				set_position_attribute(mh, name, true);
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceDeformation::linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		if (parameter_set_.count(mh) > 0ul)
		{
			MapParameters& p = parameter_set_[mh];
			if (QString::fromStdString(p.position_.name()) == name)
				set_position_attribute(mh, "", true);
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

void Plugin_SurfaceDeformation::linked_map_cells_set_removed(cgogn::Orbit orbit, const QString& name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(sender());

	if (orbit == CMap2::Vertex::ORBIT)
	{
		if (parameter_set_.count(mh) > 0ul)
		{
			MapParameters& p = parameter_set_[mh];
			CMapCellsSetGen* fvs = p.free_vertex_set();
			if (fvs && fvs->name() == name)
				set_free_vertex_set(mh, nullptr, true);
			CMapCellsSetGen* hvs = p.handle_vertex_set();
			if (hvs && hvs->name() == name)
				set_handle_vertex_set(mh, nullptr, true);
		}

		for (View* view : mh->linked_views())
			view->update();
	}
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceDeformation::set_position_attribute(CMap2Handler* mh, const QString& name, bool update_dock_tab)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		bool success = p.set_position_attribute(name);
		if (update_dock_tab && dock_tab_->selected_map() == mh)
		{
			if (success)
				dock_tab_->set_position_attribute(name);
			else
				dock_tab_->set_position_attribute("");
		}
		stop(mh, true);
	}
}

void Plugin_SurfaceDeformation::set_free_vertex_set(CMap2Handler* mh, CMap2CellsSet<CMap2::Vertex>* cs, bool update_dock_tab)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		bool success = p.set_free_vertex_set(cs);
		if (update_dock_tab && dock_tab_->selected_map() == mh)
		{
			if (success)
				dock_tab_->set_free_vertex_set(cs);
			else
				dock_tab_->set_free_vertex_set(nullptr);
		}
		stop(mh, true);
	}
}

void Plugin_SurfaceDeformation::set_handle_vertex_set(CMap2Handler* mh, CMap2CellsSet<CMap2::Vertex>* cs, bool update_dock_tab)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		bool success = p.set_handle_vertex_set(cs);
		if (update_dock_tab && dock_tab_->selected_map() == mh)
		{
			if (success)
				dock_tab_->set_handle_vertex_set(cs);
			else
				dock_tab_->set_handle_vertex_set(nullptr);
		}
		stop(mh, true);
	}
}

void Plugin_SurfaceDeformation::initialize(CMap2Handler* mh, bool update_dock_tab)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		if (!p.initialized_)
		{
			p.initialize();
			if (update_dock_tab && dock_tab_->selected_map() == mh)
				dock_tab_->set_deformation_initialized(p.initialized_);
		}
	}
}

void Plugin_SurfaceDeformation::stop(CMap2Handler* mh, bool update_dock_tab)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		if (p.initialized_)
		{
			p.stop();
			if (update_dock_tab && dock_tab_->selected_map() == mh)
				dock_tab_->set_deformation_initialized(p.initialized_);
		}
	}
}

bool Plugin_SurfaceDeformation::as_rigid_as_possible(CMap2Handler* mh)
{
	if (mh)
	{
		MapParameters& p = parameters(mh);
		if (p.initialized_)
		{
			if (!p.solver_ready_)
				if (!p.build_solver())
					return false;

			CMap2* map = p.mh_->map();

			auto compute_rotation_matrix = [&] (CMap2::Vertex v)
			{
				MAT33 cov;
				cov.setZero();
				const VEC3& pos = p.position_[v];
				const VEC3& pos_i = p.position_init_[v];
				map->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
				{
					VEC3 vec = p.position_[av] - pos;
					VEC3 vec_i = p.position_init_[av] - pos_i;
					for (uint32 i = 0; i < 3; ++i)
						for (uint32 j = 0; j < 3; ++j)
							cov(i,j) += vec[i] * vec_i[j];
				});
				Eigen::JacobiSVD<MAT33> svd(cov, Eigen::ComputeFullU | Eigen::ComputeFullV);
				MAT33 R = svd.matrixU() * svd.matrixV().transpose();
				if (R.determinant() < 0)
				{
					MAT33 U = svd.matrixU();
					for (uint32 i = 0; i < 3; ++i)
						U(i,2) *= -1;
					R = U * svd.matrixV().transpose();
				}
				p.vertex_rotation_matrix_[v] = R;
			};
			map->parallel_foreach_cell(compute_rotation_matrix, *p.working_cells_);

			auto compute_rotated_diff_coord = [&] (CMap2::Vertex v)
			{
				uint32 degree = 0;
				MAT33 r;
				r.setZero();
				map->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
				{
					r += p.vertex_rotation_matrix_[av];
					++degree;
				});
				r += p.vertex_rotation_matrix_[v];
				r /= degree + 1;
				p.rotated_diff_coord_[v] = r * p.diff_coord_[v];
			};
			map->parallel_foreach_cell(compute_rotated_diff_coord, *p.working_cells_);

			uint32 nb_vertices = p.working_cells_->size<CMap2::Vertex>();

			Eigen::MatrixXd rdiff(nb_vertices, 3);
			map->parallel_foreach_cell(
				[&] (CMap2::Vertex v)
				{
					const VEC3& rdcv = p.rotated_diff_coord_[v];
					uint32 vidx = p.v_index_[v];
					rdiff(vidx, 0) = rdcv[0];
					rdiff(vidx, 1) = rdcv[1];
					rdiff(vidx, 2) = rdcv[2];
				},
				*p.working_cells_
			);
			Eigen::MatrixXd rbdiff(nb_vertices, 3);
			rbdiff = p.working_LAPL_ * rdiff;
			map->parallel_foreach_cell(
				[&] (CMap2::Vertex v)
				{
					VEC3& rbdcv = p.rotated_bi_diff_coord_[v];
					uint32 vidx = p.v_index_[v];
					rbdcv[0] = rbdiff(vidx, 0);
					rbdcv[1] = rbdiff(vidx, 1);
					rbdcv[2] = rbdiff(vidx, 2);
				},
				*p.working_cells_
			);

			Eigen::MatrixXd x(nb_vertices, 3);
			Eigen::MatrixXd b(nb_vertices, 3);

			map->parallel_foreach_cell(
				[&] (CMap2::Vertex v)
				{
					uint32 vidx = p.v_index_[v];
					if (p.free_vertex_set_->is_selected(v))
					{
						const VEC3& rbdc = p.rotated_bi_diff_coord_[v];
						b.coeffRef(vidx, 0) = rbdc[0];
						b.coeffRef(vidx, 1) = rbdc[1];
						b.coeffRef(vidx, 2) = rbdc[2];
					}
					else
					{
						const VEC3& pos = p.position_[v];
						b.coeffRef(vidx, 0) = pos[0];
						b.coeffRef(vidx, 1) = pos[1];
						b.coeffRef(vidx, 2) = pos[2];
					}
				},
				*p.working_cells_
			);

			x = p.solver_->solve(b);

			map->parallel_foreach_cell(
				[&] (CMap2::Vertex v)
				{
					uint32 vidx = p.v_index_[v];
					VEC3& pos = p.position_[v];
					pos[0] = x(vidx, 0);
					pos[1] = x(vidx, 1);
					pos[2] = x(vidx, 2);
				},
				*p.working_cells_
			);

			return true;
		}
		else
		{
			cgogn_log_warning("surface_deformation") << "initial state not initialized";
			return false;
		}
	}

	cgogn_log_warning("surface_deformation") << "wrong map dimension";
	return false;
}

} // namespace plugin_surface_deformation

} // namespace schnapps
