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

#include <thirdparty/OpenNL/OpenNL_psm.h>

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
		nb_vertices_(0),
		solver(nullptr)
	{}

	~MapParameters()
	{}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	const CMap2::VertexAttribute<VEC3>& get_position_attribute() const { return position_; }
	QString get_position_attribute_name() const { return QString::fromStdString(position_.name()); }
	CMap2Handler::VertexSet* get_free_vertex_set() const { return free_vertex_set_; }
	CMap2Handler::VertexSet* get_handle_vertex_set() const { return handle_vertex_set_; }
	bool get_initialized() const { return initialized_; }

	void start_stop()
	{
		if (!initialized_)
		{
			if (position_.is_valid() &&
				free_vertex_set_ && free_vertex_set_->get_nb_cells() > 0 &&
				handle_vertex_set_ && handle_vertex_set_->get_nb_cells() > 0)
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

				CMap2::CellMarker<CMap2::Vertex::ORBIT> working_vertices_marker(*map2);
				CMap2::CellMarker<CMap2::Edge::ORBIT> working_edges_marker(*map2);

				map2->copy_attribute(position_init_, position_);

				v_index_.set_all_values(-1);

				MAT33 m;
				m.setZero();
				vertex_rotation_matrix_.set_all_values(m);

				// compute edges weight
				auto compute_edge_weight = [&] (CMap2::Edge e)
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
				};
				map2->parallel_foreach_cell(compute_edge_weight);

				// build the cell cache of working area vertices (and mark them)
				working_cells_->build<CMap2::Vertex>([&] (CMap2::Vertex v) -> bool
				{
					if (handle_vertex_set_->is_selected(v)) // handle vertices
					{
						// TODO: check that it is surrounded by only handle or free vertices
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

				// build the cell cache of working area edges (and mark them)
				working_cells_->build<CMap2::Edge>([&] (CMap2::Edge e) -> bool
				{
					auto vertices = map2->vertices(e);
					if (
						working_vertices_marker.is_marked(vertices.first) &&
						working_vertices_marker.is_marked(vertices.second)
					)
					{
						working_edges_marker.mark(e);
						return true;
					}
					return false;
				});

				// index the working area vertices
				nb_vertices_ = 0;
				// start with the free vertices
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						if (free_vertex_set_->is_selected(v))
							v_index_[v] = nb_vertices_++;
					},
					*working_cells_
				);
				// then the others (handle & area boundary <=> constrained)
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						if (!free_vertex_set_->is_selected(v))
							v_index_[v] = nb_vertices_++;
					},
					*working_cells_
				);

				// init laplacian matrix
				L.setZero();
				L.resize(nb_vertices_, nb_vertices_);

				std::vector<Eigen::Triplet<SCALAR>> coeffs;
				coeffs.reserve(nb_vertices_ * 8);

				auto add_edge = [&] (CMap2::Edge e)
				{
					SCALAR w = edge_weight_[e];
					auto vertices = map2->vertices(e);

					coeffs.push_back(Eigen::Triplet<SCALAR>(v_index_[vertices.first], v_index_[vertices.second], w));
					coeffs.push_back(Eigen::Triplet<SCALAR>(v_index_[vertices.second], v_index_[vertices.first], w));

					coeffs.push_back(Eigen::Triplet<SCALAR>(v_index_[vertices.first], v_index_[vertices.first], -w));
					coeffs.push_back(Eigen::Triplet<SCALAR>(v_index_[vertices.second], v_index_[vertices.second], -w));
				};
				map2->foreach_cell(add_edge, *working_cells_);

				L.setFromTriplets(coeffs.begin(), coeffs.end());

				// init a position matrix for laplacian computation
				Eigen::MatrixXd pos(nb_vertices_, 3);
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						const VEC3& pv = position_[v];
						pos(v_index_[v], 0) = pv[0];
						pos(v_index_[v], 1) = pv[1];
						pos(v_index_[v], 2) = pv[2];
					},
					*working_cells_
				);

				// compute vertices laplacian
				Eigen::MatrixXd diff(nb_vertices_, 3);
				diff = L * pos;
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						VEC3& dcv = diff_coord_[v];
						dcv[0] = diff(v_index_[v], 0);
						dcv[1] = diff(v_index_[v], 1);
						dcv[2] = diff(v_index_[v], 2);
					},
					*working_cells_
				);

				// init bi-Laplacian matrix
				L2.setZero();
				L2.resize(nb_vertices_, nb_vertices_);

				Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> Lcopy1(L);
				Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> Lcopy2(L);
				L2 = Lcopy1 * Lcopy2;

				// compute vertices bi-laplacian
				Eigen::MatrixXd bi_diff(nb_vertices_, 3);
				bi_diff = L2 * pos;
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						VEC3& bdcv = bi_diff_coord_[v];
						bdcv[0] = bi_diff(v_index_[v], 0);
						bdcv[1] = bi_diff(v_index_[v], 1);
						bdcv[2] = bi_diff(v_index_[v], 2);
					},
					*working_cells_
				);

				// set contrained vertices
				map2->foreach_cell(
					[&] (CMap2::Vertex v)
					{
						if (!free_vertex_set_->is_selected(v))
						{
							uint32 idx = v_index_[v];
							L.prune([&] (int i, int, SCALAR) { return i != idx; });
							L.coeffRef(idx, idx) = 1.0;
							L2.prune([&] (int i, int, SCALAR) { return i != idx; });
							L2.coeffRef(idx, idx) = 1.0;
						}
					},
					*working_cells_
				);

				L.makeCompressed();
				L2.makeCompressed();

				if (solver)
					delete solver;

				solver = new Eigen::SparseLU<Eigen::SparseMatrix<SCALAR, Eigen::ColMajor>>(L2);
				solver->analyzePattern(L2);
				solver->factorize(L2);

				initialized_ = true;
			}
		}
		else
		{
			initialized_ = false;
		}
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
		if (cs && cs->get_map() == map_ && cs->get_cell_type() == Vertex_Cell)
		{
			free_vertex_set_ = static_cast<CMap2Handler::VertexSet*>(cs);
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
		if (cs && cs->get_map() == map_ && cs->get_cell_type() == Vertex_Cell)
		{
			handle_vertex_set_ = static_cast<CMap2Handler::VertexSet*>(cs);
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

	bool initialized_;

	CMap2::VertexAttribute<VEC3> position_init_;
	CMap2::VertexAttribute<VEC3> diff_coord_;
	CMap2::VertexAttribute<VEC3> bi_diff_coord_;
	CMap2::VertexAttribute<MAT33> vertex_rotation_matrix_;
	CMap2::VertexAttribute<VEC3> rotated_diff_coord_;
	CMap2::VertexAttribute<VEC3> rotated_bi_diff_coord_;

	CMap2::EdgeAttribute<SCALAR> edge_weight_;

	CMap2::VertexAttribute<uint32> v_index_;
	uint32 nb_vertices_;

	Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> L;
	Eigen::SparseMatrix<SCALAR, Eigen::ColMajor> L2;
	Eigen::SparseLU<Eigen::SparseMatrix<SCALAR, Eigen::ColMajor>>* solver;
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_MAP_PARAMETERS_H_
