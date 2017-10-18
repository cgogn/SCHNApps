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

#include <surface_deformation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

MapParameters& Plugin_SurfaceDeformation::get_parameters(MapHandlerGen* map)
{
	cgogn_message_assert(map, "Try to access parameters for null map");
	cgogn_message_assert(map->dimension() == 2, "Try to access parameters for map with dimension other than 2");

	if (parameter_set_.count(map) == 0)
	{
		MapParameters& p = parameter_set_[map];
		p.map_ = static_cast<CMap2Handler*>(map);
		p.free_boundary_mark_ = cgogn::make_unique<CMap2::CellMarker<CMap2::Vertex::ORBIT>>(*p.map_->get_map());
		return p;
	}
	else
		return parameter_set_[map];
}

bool Plugin_SurfaceDeformation::check_docktab_activation()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (view && view->is_linked_to_plugin(this) && map && map->is_linked_to_view(view) && map->dimension() == 2)
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
	if (get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = get_setting("Auto load position attribute").toString();
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
	switch (event->key())
	{
		case Qt::Key_D: {
			MapHandlerGen* mhg = schnapps_->get_selected_map();
			if (mhg && mhg->dimension() == 2)
			{
				const MapParameters& p = get_parameters(mhg);
				if (!dragging_ && p.get_handle_vertex_set() && p.get_handle_vertex_set()->get_nb_cells() > 0)
				{
					dragging_ = true;
					drag_init_ = false;
					view->setMouseTracking(true);
					view->get_current_camera()->disable_views_bb_fitting();
				}
				else
				{
					dragging_ = false;
					drag_init_ = false;
					view->setMouseTracking(false);
					view->get_current_camera()->enable_views_bb_fitting();
				}
			}
			break;
		}

		case Qt::Key_R : {
			MapHandlerGen* mhg = schnapps_->get_selected_map();
			if (mhg && mhg->dimension() == 2)
			{
				const MapParameters& p = get_parameters(mhg);
				if (p.initialized_)
				{
					as_rigid_as_possible(mhg);
					mhg->notify_attribute_change(CMap2::Vertex::ORBIT, p.get_position_attribute_name());

					for (View* view : mhg->get_linked_views())
						view->update();
				}
			}
			break;
		}
	}

	return true;
}

bool Plugin_SurfaceDeformation::mouseMove(View* view, QMouseEvent* event)
{
	if (dragging_)
	{
		MapHandlerGen* mhg = schnapps_->get_selected_map();
		if (mhg && mhg->dimension() == 2)
		{
			MapParameters& p = get_parameters(mhg);
			if (!drag_init_)
			{
				drag_z_ = 0;
				p.handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					const VEC3& pp = p.position_[v];
					qoglviewer::Vec q = view->camera()->projectedCoordinatesOf(qoglviewer::Vec(pp[0],pp[1],pp[2]));
					drag_z_ += q.z;
				});
				drag_z_ /= p.handle_vertex_set_->get_nb_cells();

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
					as_rigid_as_possible(mhg);

				mhg->notify_attribute_change(CMap2::Vertex::ORBIT, p.get_position_attribute_name());

				for (View* view : mhg->get_linked_views())
					view->update();
			}
		}
	}

	return true;
}

void Plugin_SurfaceDeformation::view_linked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	connect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	connect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { add_linked_map(view, map); }
}

void Plugin_SurfaceDeformation::view_unlinked(View* view)
{
	if (check_docktab_activation())
		dock_tab_->refresh_ui();

	disconnect(view, SIGNAL(map_linked(MapHandlerGen*)), this, SLOT(map_linked(MapHandlerGen*)));
	disconnect(view, SIGNAL(map_unlinked(MapHandlerGen*)), this, SLOT(map_unlinked(MapHandlerGen*)));

	for (MapHandlerGen* map : view->get_linked_maps()) { remove_linked_map(view, map); }
}

void Plugin_SurfaceDeformation::map_linked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	add_linked_map(view, map);
}

void Plugin_SurfaceDeformation::add_linked_map(View* view, MapHandlerGen* map)
{
	MapParameters& p = get_parameters(map);
	p.set_position_attribute(setting_auto_load_position_attribute_);

	connect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	connect(map, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(linked_map_cells_set_removed(CellType, const QString&)), Qt::UniqueConnection);

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_SurfaceDeformation::map_unlinked(MapHandlerGen *map)
{
	View* view = static_cast<View*>(sender());
	remove_linked_map(view, map);
}

void Plugin_SurfaceDeformation::remove_linked_map(View* view, MapHandlerGen* map)
{
	disconnect(map, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_added(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(linked_map_attribute_removed(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(linked_map_cells_set_removed(CellType, const QString&)));

	if (check_docktab_activation())
		dock_tab_->refresh_ui();
}

void Plugin_SurfaceDeformation::linked_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		if (parameter_set_.count(map) > 0ul)
		{
			MapParameters& p = parameter_set_[map];
			if (!p.get_position_attribute().is_valid() && name == setting_auto_load_position_attribute_)
				set_position_attribute(map, name, true);
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceDeformation::linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		if (parameter_set_.count(map) > 0ul)
		{
			MapParameters& p = parameter_set_[map];
			if (p.get_position_attribute_name() == name)
				set_position_attribute(map, "", true);
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

void Plugin_SurfaceDeformation::linked_map_cells_set_removed(CellType ct, const QString& name)
{
	if (ct == Vertex_Cell)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

		if (parameter_set_.count(map) > 0ul)
		{
			MapParameters& p = parameter_set_[map];
			CellsSetGen* fvs = p.get_free_vertex_set();
			if (fvs && fvs->get_name() == name)
				set_free_vertex_set(map, nullptr, true);
			CellsSetGen* hvs = p.get_handle_vertex_set();
			if (hvs && hvs->get_name() == name)
				set_handle_vertex_set(map, nullptr, true);
		}

		for (View* view : map->get_linked_views())
			view->update();
	}
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_SurfaceDeformation::set_position_attribute(MapHandlerGen* map, const QString& name, bool update_dock_tab)
{
	if (map && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(map);
		bool success = p.set_position_attribute(name);
		if (update_dock_tab && map->is_selected_map())
		{
			if (success)
				dock_tab_->set_position_attribute(name);
			else
				dock_tab_->set_position_attribute("");
		}
	}
}

void Plugin_SurfaceDeformation::set_free_vertex_set(MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab)
{
	if (map && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(map);
		bool success = p.set_free_vertex_set(cs);
		if (update_dock_tab && map->is_selected_map())
		{
			if (success)
				dock_tab_->set_free_vertex_set(cs);
			else
				dock_tab_->set_free_vertex_set(nullptr);
		}
	}
}

void Plugin_SurfaceDeformation::set_handle_vertex_set(MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab)
{
	if (map && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(map);
		bool success = p.set_handle_vertex_set(cs);
		if (update_dock_tab && map->is_selected_map())
		{
			if (success)
				dock_tab_->set_handle_vertex_set(cs);
			else
				dock_tab_->set_handle_vertex_set(nullptr);
		}
	}
}

void Plugin_SurfaceDeformation::start_stop(MapHandlerGen* map, bool update_dock_tab)
{
	if (map && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(map);
		p.start_stop();
		if (update_dock_tab && map->is_selected_map())
			dock_tab_->set_deformation_initialized(p.initialized_);
	}
}

void Plugin_SurfaceDeformation::as_rigid_as_possible(MapHandlerGen* map)
{
	if (map && map->dimension() == 2)
	{
		MapParameters& p = get_parameters(map);
		if (p.initialized_)
		{
			CMap2* map2 = p.map_->get_map();

			auto compute_rotation_matrix = [&] (CMap2::Vertex v)
			{
				MAT33 cov;
				cov.setZero();
				const VEC3& pos = p.position_[v];
				const VEC3& pos_i = p.position_init_[v];
				map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
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
			p.free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
			{
				if (!p.free_boundary_mark_->is_marked(v))
					compute_rotation_matrix(v);
			});
			p.handle_vertex_set_->foreach_cell(compute_rotation_matrix);

			auto compute_rotated_diff_coord = [&] (CMap2::Vertex v)
			{
				uint32 degree = 0;
				MAT33 r;
				r.setZero();
				map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
				{
					r += p.vertex_rotation_matrix_[av];
					++degree;
				});
				r += p.vertex_rotation_matrix_[v];
				r /= degree + 1;
				const VEC3& dc = p.diff_coord_[v];
				p.rotated_diff_coord_[v] = r * dc;
			};
			p.free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
			{
				if (!p.free_boundary_mark_->is_marked(v))
					compute_rotated_diff_coord(v);
			});
			p.handle_vertex_set_->foreach_cell(compute_rotated_diff_coord);

			for (uint32 coord = 0; coord < 3; ++coord)
			{
				NLContext context = nlNewContext();
				nlSolverParameteri(NL_NB_VARIABLES, p.nb_vertices_);
				nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);

				nlBegin(NL_SYSTEM);

				p.free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					nlSetVariable(p.v_index_[v], p.position_[v][coord]);
					if (p.free_boundary_mark_->is_marked(v))
						nlLockVariable(p.v_index_[v]);
				});
				p.handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					nlSetVariable(p.v_index_[v], p.position_[v][coord]);
					nlLockVariable(p.v_index_[v]);
				});

				auto add_row = [&] (CMap2::Vertex v)
				{
					std::vector<std::pair<uint32, SCALAR>> coeffs; coeffs.reserve(16);
					SCALAR norm2(0);
					SCALAR aii(0);
					map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
					{
						SCALAR aij(1);
						aii += aij;
						coeffs.push_back(std::make_pair(p.v_index_[av], aij));
						norm2 += aij * aij;
					});
					coeffs.push_back(std::make_pair(p.v_index_[v], -aii));
					norm2 += aii * aii;

					nlRightHandSide(p.rotated_diff_coord_[v][coord] * std::sqrt(norm2));
					nlBegin(NL_ROW);
					for (const auto& c : coeffs)
						nlCoefficient(c.first, c.second);
					nlEnd(NL_ROW);
				};

				nlBegin(NL_MATRIX);
				nlEnable(NL_NORMALIZE_ROWS);
				p.free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					if (!p.free_boundary_mark_->is_marked(v))
						add_row(v);
				});
				p.handle_vertex_set_->foreach_cell(add_row);
				nlDisable(NL_NORMALIZE_ROWS);
				nlEnd(NL_MATRIX);

				nlEnd(NL_SYSTEM);

				nlSolve();

				p.free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					if (!p.free_boundary_mark_->is_marked(v))
						(p.position_[v])[coord] = SCALAR(nlGetVariable(p.v_index_[v]));
				});

				nlDeleteContext(context);
			}
		}
	}
}

} // namespace plugin_surface_deformation

} // namespace schnapps
