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

#ifndef SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_MAP_PARAMETERS_H_
#define SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_MAP_PARAMETERS_H_

#include <schnapps/plugins/surface_deformation/dll.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>
#include <schnapps/plugins/cmap2_provider/cmap2_cells_set.h>

#include <schnapps/core/types.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/area.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

class Plugin_SurfaceDeformation;
using CMap2Handler = plugin_cmap2_provider::CMap2Handler;
template <typename CELL>
using CMap2CellsSet = plugin_cmap2_provider::CMap2CellsSet<CELL>;
using CMap2CellsSetGen = plugin_cmap2_provider::CMap2CellsSetGen;

struct MapParameters
{
	friend class Plugin_SurfaceDeformation;

	MapParameters() :
		mh_(nullptr),
		free_vertex_set_(nullptr),
		handle_vertex_set_(nullptr),
		initialized_(false),
		solver_ready_(false),
		solver_(nullptr)
	{}

	~MapParameters()
	{
		if (solver_)
			delete solver_;
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	const CMap2::VertexAttribute<VEC3>& position_attribute() const { return position_; }
	CMap2CellsSet<CMap2::Vertex>* free_vertex_set() const { return free_vertex_set_; }
	CMap2CellsSet<CMap2::Vertex>* handle_vertex_set() const { return handle_vertex_set_; }
	bool initialized() const { return initialized_; }

	void initialize()
	{
		if (!initialized_ && position_.is_valid())
		{
			CMap2* map = mh_->map();

			position_init_ = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>("position_init");
			if (!position_init_.is_valid())
				position_init_ = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position_init");

			diff_coord_ = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>("diff_coord");
			if (!diff_coord_.is_valid())
				diff_coord_ = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("diff_coord");

			bi_diff_coord_ = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>("bi_diff_coord");
			if (!bi_diff_coord_.is_valid())
				bi_diff_coord_ = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("bi_diff_coord");

			vertex_rotation_matrix_ = map->get_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");
			if (!vertex_rotation_matrix_.is_valid())
				vertex_rotation_matrix_ = map->add_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");

			rotated_diff_coord_ = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");
			if (!rotated_diff_coord_.is_valid())
				rotated_diff_coord_ = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");

			rotated_bi_diff_coord_ = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_bi_diff_coord");
			if (!rotated_bi_diff_coord_.is_valid())
				rotated_bi_diff_coord_ = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_bi_diff_coord");

			edge_weight_ = map->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_weight");
			if (!edge_weight_.is_valid())
				edge_weight_ = map->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_weight");

			v_index_ = map->get_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");
			if (!v_index_.is_valid())
				v_index_ = map->add_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");

			// initialize position init values
			map->copy_attribute(position_init_, position_);

			// initialize vertex rotation matrix
			MAT33 m;
			m.setZero();
			vertex_rotation_matrix_.set_all_values(m);

			// compute edges weight
			map->parallel_foreach_cell([&] (CMap2::Edge e)
			{
				if (!map->is_incident_to_boundary(e))
				{
					edge_weight_[e] = (
						std::tan(M_PI_2 - cgogn::geometry::angle(*map, CMap2::CDart(map->phi_1(e.dart)), position_)) +
						std::tan(M_PI_2 - cgogn::geometry::angle(*map, CMap2::CDart(map->phi_1(map->phi2(e.dart))), position_))
					) / 2.0;
				}
				else
				{
					cgogn::Dart d = map->boundary_dart(e);
					edge_weight_[e] = std::tan(M_PI_2 - cgogn::geometry::angle(*map, CMap2::CDart(map->phi_1(map->phi2(d))), position_));
				}
			});

			// compute vertices laplacian
			uint32 nb_vertices = 0;
			map->foreach_cell([&] (CMap2::Vertex v)
			{
				v_index_[v] = nb_vertices++;
			});
			Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> LAPL(nb_vertices, nb_vertices);
			std::vector<Eigen::Triplet<SCALAR>> LAPLcoeffs;
			LAPLcoeffs.reserve(nb_vertices * 10);
			map->foreach_cell([&] (CMap2::Edge e)
			{
				SCALAR w = edge_weight_[e];
				auto vertices = map->vertices(e);
				uint32 vidx1 = v_index_[vertices.first];
				uint32 vidx2 = v_index_[vertices.second];

				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx2, w));
				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx1, w));

				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx1, -w));
				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx2, -w));
			});
			LAPL.setFromTriplets(LAPLcoeffs.begin(), LAPLcoeffs.end());
			Eigen::MatrixXd vpos(nb_vertices, 3);
			map->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				const VEC3& pv = position_[v];
				uint32 vidx = v_index_[v];
				vpos(vidx, 0) = pv[0];
				vpos(vidx, 1) = pv[1];
				vpos(vidx, 2) = pv[2];
			});
			Eigen::MatrixXd lapl(nb_vertices, 3);
			lapl = LAPL * vpos;
			map->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				VEC3& dcv = diff_coord_[v];
				uint32 vidx = v_index_[v];
				dcv[0] = lapl(vidx, 0);
				dcv[1] = lapl(vidx, 1);
				dcv[2] = lapl(vidx, 2);
			});

			// compute vertices bi-laplacian
			Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> BILAPL(nb_vertices, nb_vertices);
			BILAPL = LAPL * LAPL;
			Eigen::MatrixXd bilapl(nb_vertices, 3);
			bilapl = BILAPL * vpos;
			map->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				VEC3& bdcv = bi_diff_coord_[v];
				uint32 vidx = v_index_[v];
				bdcv[0] = bilapl(vidx, 0);
				bdcv[1] = bilapl(vidx, 1);
				bdcv[2] = bilapl(vidx, 2);
			});

			initialized_ = true;
			solver_ready_ = false;
		}
	}

	void stop()
	{
		if (solver_)
		{
			delete solver_;
			solver_ = nullptr;
		}

		working_cells_->clear<CMap2::Vertex>();
		working_cells_->clear<CMap2::Edge>();

		working_LAPL_.setZero();
		working_BILAPL_.setZero();

		initialized_ = false;
		solver_ready_ = false;
	}

	bool build_solver()
	{
		if (initialized_ && !solver_ready_ &&
			free_vertex_set_ && free_vertex_set_->nb_cells() > 0 &&
			handle_vertex_set_ && handle_vertex_set_->nb_cells() > 0)
		{
			CMap2* map = mh_->map();
			CMap2::CellMarkerStore<CMap2::Vertex::ORBIT> working_vertices_marker(*map);

			// check that handle vertices are surrounded only by handle or free vertices
			bool handle_ok = true;
			map->foreach_cell([&] (CMap2::Vertex v) -> bool
			{
				if (handle_vertex_set_->is_selected(v))
				{
					map->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
					{
						if (!handle_vertex_set_->is_selected(av) && !free_vertex_set_->is_selected(av))
							handle_ok = false;
						return handle_ok;
					});
				}
				return handle_ok;
			});
			if (!handle_ok)
			{
				cgogn_log_warning("surface_deformation") << "Handle is not defined in the free area";
				return false;
			}

			// build the cell cache of working area vertices (and mark them)
			working_cells_->build<CMap2::Vertex>([&] (CMap2::Vertex v) -> bool
			{
				if (handle_vertex_set_->is_selected(v)) // handle vertices
				{
					working_vertices_marker.mark(v);
					return true;
				}
				if (free_vertex_set_->is_selected(v)) // free vertices
				{
					working_vertices_marker.mark(v);
					map->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) // and their 2-ring
					{
						if (!free_vertex_set_->is_selected(av) &&
							!handle_vertex_set_->is_selected(av) &&
							!working_vertices_marker.is_marked(av))
						{
							working_cells_->add(av);
							working_vertices_marker.mark(av);
							map->foreach_adjacent_vertex_through_edge(av, [&] (CMap2::Vertex aav)
							{

								if (!free_vertex_set_->is_selected(aav) &&
									!handle_vertex_set_->is_selected(aav) &&
									!working_vertices_marker.is_marked(aav))
								{
									working_cells_->add(aav);
									working_vertices_marker.mark(aav);
								}
							});
						}
					});
					return true;
				}
				return false;
			});

			// build the cell cache of working area edges
			working_cells_->build<CMap2::Edge>([&] (CMap2::Edge e) -> bool
			{
				auto vertices = map->vertices(e);
				return (
					working_vertices_marker.is_marked(vertices.first) &&
					working_vertices_marker.is_marked(vertices.second)
				);
			});

			// index the working area vertices
			uint32 nb_vertices = 0;
			// start with the free vertices
			map->foreach_cell(
				[&] (CMap2::Vertex v)
				{
					if (free_vertex_set_->is_selected(v))
						v_index_[v] = nb_vertices++;
				},
				*working_cells_
			);
			// then the others (handle & area boundary <=> constrained)
			map->foreach_cell(
				[&] (CMap2::Vertex v)
				{
					if (!free_vertex_set_->is_selected(v))
						v_index_[v] = nb_vertices++;
				},
				*working_cells_
			);

			// init laplacian matrix
			working_LAPL_.setZero();
			working_LAPL_.resize(nb_vertices, nb_vertices);
			std::vector<Eigen::Triplet<SCALAR>> LAPLcoeffs;
			LAPLcoeffs.reserve(nb_vertices * 10);
			map->foreach_cell(
				[&] (CMap2::Edge e)
				{
					SCALAR w = edge_weight_[e];
					auto vertices = map->vertices(e);
					uint32 vidx1 = v_index_[vertices.first];
					uint32 vidx2 = v_index_[vertices.second];

					LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx2, w));
					LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx1, w));

					LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx1, -w));
					LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx2, -w));
				},
				*working_cells_
			);
			working_LAPL_.setFromTriplets(LAPLcoeffs.begin(), LAPLcoeffs.end());

			// init bi-laplacian matrix
			working_BILAPL_.setZero();
			working_BILAPL_.resize(nb_vertices, nb_vertices);
			working_BILAPL_ = working_LAPL_ * working_LAPL_;

			// set contrained vertices
			map->foreach_cell(
				[&] (CMap2::Vertex v)
				{
					if (!free_vertex_set_->is_selected(v))
					{
						uint32 idx = v_index_[v];
						working_LAPL_.prune([&] (int i, int, SCALAR) { return i != idx; });
						working_LAPL_.coeffRef(idx, idx) = 1.0;
						working_BILAPL_.prune([&] (int i, int, SCALAR) { return i != idx; });
						working_BILAPL_.coeffRef(idx, idx) = 1.0;
					}
				},
				*working_cells_
			);

			working_LAPL_.makeCompressed();
			working_BILAPL_.makeCompressed();

			if (solver_)
				delete solver_;
			solver_ = new Eigen::SparseLU<Eigen::SparseMatrix<SCALAR, Eigen::ColMajor>>(working_BILAPL_);

			solver_ready_ = true;
		}

		return solver_ready_;
	}

private:

	bool set_position_attribute(const QString& attribute_name)
	{
		position_ = mh_->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
		if (position_.is_valid())
			return true;
		else
			return false;
	}

	bool set_free_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs)
	{
		if (free_vertex_set_)
			QObject::disconnect(fvs_connection_);
		if (cs && &cs->map_handler() == mh_)
		{
			free_vertex_set_ = cs;
			fvs_connection_ = QObject::connect(free_vertex_set_, &CMap2CellsSetGen::selected_cells_changed, [this] () { solver_ready_ = false; });
			return true;
		}
		else
		{
			free_vertex_set_ = nullptr;
			return false;
		}
	}

	bool set_handle_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs)
	{
		if (handle_vertex_set_)
			QObject::disconnect(hvs_connection_);
		if (cs && &cs->map_handler() == mh_)
		{
			handle_vertex_set_ = cs;
			hvs_connection_ = QObject::connect(handle_vertex_set_, &CMap2CellsSetGen::selected_cells_changed, [this] () { solver_ready_ = false; });
			return true;
		}
		else
		{
			handle_vertex_set_ = nullptr;
			return false;
		}
	}

	CMap2Handler* mh_;
	std::unique_ptr<CMap2::CellCache> working_cells_;

	CMap2::VertexAttribute<VEC3> position_;

	CMap2CellsSet<CMap2::Vertex>* free_vertex_set_;
	CMap2CellsSet<CMap2::Vertex>* handle_vertex_set_;
	QMetaObject::Connection fvs_connection_;
	QMetaObject::Connection hvs_connection_;

	Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> working_LAPL_;
	Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> working_BILAPL_;

	bool initialized_;
	bool solver_ready_;

	CMap2::VertexAttribute<VEC3> position_init_;
	CMap2::VertexAttribute<VEC3> diff_coord_;
	CMap2::VertexAttribute<VEC3> bi_diff_coord_;
	CMap2::VertexAttribute<MAT33> vertex_rotation_matrix_;
	CMap2::VertexAttribute<VEC3> rotated_diff_coord_;
	CMap2::VertexAttribute<VEC3> rotated_bi_diff_coord_;
	CMap2::VertexAttribute<uint32> v_index_;

	CMap2::EdgeAttribute<SCALAR> edge_weight_;

	Eigen::SparseLU<Eigen::SparseMatrix<SCALAR, Eigen::ColMajor>>* solver_;
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_MAP_PARAMETERS_H_
