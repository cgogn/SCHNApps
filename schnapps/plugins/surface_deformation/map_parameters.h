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

#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/area.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

class Plugin_SurfaceDeformation;

struct MapParameters
{
	friend class Plugin_SurfaceDeformation;

	MapParameters() :
		map_(nullptr),
		free_vertex_set_(nullptr),
		handle_vertex_set_(nullptr),
		initialized_(false),
		solver_ready_(false),
		solver_(nullptr)
	{}

	~MapParameters()
	{}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	const CMap2::VertexAttribute<VEC3>& get_position_attribute() const { return position_; }
	QString get_position_attribute_name() const { return QString::fromStdString(position_.name()); }
	CMap2Handler::VertexSet* get_free_vertex_set() const { return free_vertex_set_; }
	CMap2Handler::VertexSet* get_handle_vertex_set() const { return handle_vertex_set_; }
	bool get_initialized() const { return initialized_; }

	void initialize()
	{
		if (!initialized_ && position_.is_valid())
		{
			position_init_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("position_init");
			if (!position_init_.is_valid())
				position_init_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position_init");

			diff_coord_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("diff_coord");
			if (!diff_coord_.is_valid())
				diff_coord_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("diff_coord");

			bi_diff_coord_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("bi_diff_coord");
			if (!bi_diff_coord_.is_valid())
				bi_diff_coord_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("bi_diff_coord");

			vertex_rotation_matrix_ = map_->get_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");
			if (!vertex_rotation_matrix_.is_valid())
				vertex_rotation_matrix_ = map_->add_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");

			rotated_diff_coord_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");
			if (!rotated_diff_coord_.is_valid())
				rotated_diff_coord_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");

			rotated_bi_diff_coord_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_bi_diff_coord");
			if (!rotated_bi_diff_coord_.is_valid())
				rotated_bi_diff_coord_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_bi_diff_coord");

			edge_weight_ = map_->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_weight");
			if (!edge_weight_.is_valid())
				edge_weight_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_weight");

			v_index_ = map_->get_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");
			if (!v_index_.is_valid())
				v_index_ = map_->add_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");

			CMap2* map2 = map_->get_map();

			// initialize position init values
			map2->copy_attribute(position_init_, position_);

			// initialize vertex rotation matrix
			MAT33 m;
			m.setZero();
			vertex_rotation_matrix_.set_all_values(m);

			// compute edges weight
			map2->parallel_foreach_cell([&] (CMap2::Edge e)
			{
				if (!map2->is_incident_to_boundary(e))
				{
					edge_weight_[e] = (
						std::tan(M_PI_2 - cgogn::geometry::angle<VEC3>(*map2, CMap2::CDart(map2->phi_1(e.dart)), position_)) +
						std::tan(M_PI_2 - cgogn::geometry::angle<VEC3>(*map2, CMap2::CDart(map2->phi_1(map2->phi2(e.dart))), position_))
					) / 2.0;
				}
				else
				{
					cgogn::Dart d = map2->boundary_dart(e);
					edge_weight_[e] = std::tan(M_PI_2 - cgogn::geometry::angle<VEC3>(*map2, CMap2::CDart(map2->phi_1(map2->phi2(e.dart))), position_));
				}
			});

			// compute vertices laplacian
			uint32 nb_vertices = 0;
			map2->foreach_cell([&] (CMap2::Vertex v)
			{
				v_index_[v] = nb_vertices++;
			});
			Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> LAPL(nb_vertices, nb_vertices);
			std::vector<Eigen::Triplet<SCALAR>> LAPLcoeffs;
			LAPLcoeffs.reserve(nb_vertices * 10);
			map2->foreach_cell([&] (CMap2::Edge e)
			{
				SCALAR w = edge_weight_[e];
				auto vertices = map2->vertices(e);
				uint32 vidx1 = v_index_[vertices.first];
				uint32 vidx2 = v_index_[vertices.second];

				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx2, w));
				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx1, w));

				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx1, vidx1, -w));
				LAPLcoeffs.push_back(Eigen::Triplet<SCALAR>(vidx2, vidx2, -w));
			});
			LAPL.setFromTriplets(LAPLcoeffs.begin(), LAPLcoeffs.end());
			Eigen::MatrixXd vpos(nb_vertices, 3);
			map2->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				const VEC3& pv = position_[v];
				vpos(v_index_[v], 0) = pv[0];
				vpos(v_index_[v], 1) = pv[1];
				vpos(v_index_[v], 2) = pv[2];
			});
			Eigen::MatrixXd lapl(nb_vertices, 3);
			lapl = LAPL * vpos;
			map2->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				VEC3& dcv = diff_coord_[v];
				dcv[0] = lapl(v_index_[v], 0);
				dcv[1] = lapl(v_index_[v], 1);
				dcv[2] = lapl(v_index_[v], 2);
			});

			// compute vertices bi-laplacian
			Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> BILAPL(nb_vertices, nb_vertices);
			BILAPL = LAPL * LAPL;
			Eigen::MatrixXd bilapl(nb_vertices, 3);
			bilapl = BILAPL * vpos;
			map2->parallel_foreach_cell([&] (CMap2::Vertex v)
			{
				VEC3& bdcv = bi_diff_coord_[v];
				bdcv[0] = bilapl(v_index_[v], 0);
				bdcv[1] = bilapl(v_index_[v], 1);
				bdcv[2] = bilapl(v_index_[v], 2);
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
			free_vertex_set_ && free_vertex_set_->get_nb_cells() > 0 &&
			handle_vertex_set_ && handle_vertex_set_->get_nb_cells() > 0)
		{
			CMap2* map2 = map_->get_map();
			CMap2::CellMarkerStore<CMap2::Vertex::ORBIT> working_vertices_marker(*map2);

			// check that handle vertices are surrounded only by handle or free vertices
			bool handle_ok = true;
			map2->foreach_cell([&] (CMap2::Vertex v) -> bool
			{
				if (handle_vertex_set_->is_selected(v))
				{
					map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
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
					map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) // and their 2-ring
					{
						if (!free_vertex_set_->is_selected(av) &&
							!handle_vertex_set_->is_selected(av) &&
							!working_vertices_marker.is_marked(av))
						{
							working_cells_->add(av);
							working_vertices_marker.mark(av);
							map2->foreach_adjacent_vertex_through_edge(av, [&] (CMap2::Vertex aav)
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
				auto vertices = map2->vertices(e);
				return (
					working_vertices_marker.is_marked(vertices.first) &&
					working_vertices_marker.is_marked(vertices.second)
				);
			});

			// index the working area vertices
			uint32 nb_vertices = 0;
			// start with the free vertices
			map2->foreach_cell(
				[&] (CMap2::Vertex v)
				{
					if (free_vertex_set_->is_selected(v))
						v_index_[v] = nb_vertices++;
				},
				*working_cells_
			);
			// then the others (handle & area boundary <=> constrained)
			map2->foreach_cell(
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
			map2->foreach_cell(
				[&] (CMap2::Edge e)
				{
					SCALAR w = edge_weight_[e];
					auto vertices = map2->vertices(e);
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
			map2->foreach_cell(
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
		position_ = map_->get_attribute<VEC3, CMap2Handler::Vertex::ORBIT>(attribute_name);
		if (position_.is_valid())
			return true;
		else
			return false;
	}

	bool set_free_vertex_set(CellsSetGen* cs)
	{
		if (free_vertex_set_)
			QObject::disconnect(fvs_connection_);
		if (cs && cs->get_map() == map_ && cs->get_cell_type() == Vertex_Cell)
		{
			free_vertex_set_ = static_cast<CMap2Handler::VertexSet*>(cs);
			fvs_connection_ = QObject::connect(free_vertex_set_, &CellsSetGen::selected_cells_changed, [this] () { solver_ready_ = false; });
			return true;
		}
		else
		{
			free_vertex_set_ = nullptr;
			return false;
		}
	}

	bool set_handle_vertex_set(CellsSetGen* cs)
	{
		if (handle_vertex_set_)
			QObject::disconnect(hvs_connection_);
		if (cs && cs->get_map() == map_ && cs->get_cell_type() == Vertex_Cell)
		{
			handle_vertex_set_ = static_cast<CMap2Handler::VertexSet*>(cs);
			hvs_connection_ = QObject::connect(handle_vertex_set_, &CellsSetGen::selected_cells_changed, [this] () { solver_ready_ = false; });
			return true;
		}
		else
		{
			handle_vertex_set_ = nullptr;
			return false;
		}
	}

	CMap2Handler* map_;
	std::unique_ptr<CMap2::CellCache> working_cells_;

	CMap2::VertexAttribute<VEC3> position_;

	CMap2Handler::VertexSet* free_vertex_set_;
	CMap2Handler::VertexSet* handle_vertex_set_;
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
