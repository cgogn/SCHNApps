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
		nb_vertices_(0)
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

				vertex_rotation_matrix_ = map_->get_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");
				if (!vertex_rotation_matrix_.is_valid())
					vertex_rotation_matrix_ = map_->add_attribute<MAT33, CMap2::Vertex::ORBIT>("vertex_rotation_matrix");

				rotated_diff_coord_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");
				if (!rotated_diff_coord_.is_valid())
					rotated_diff_coord_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("rotated_diff_coord");

				v_index_ = map_->get_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");
				if (!v_index_.is_valid())
					v_index_ = map_->add_attribute<uint32, CMap2::Vertex::ORBIT>("v_index");

				CMap2* map2 = map_->get_map();

				map2->copy_attribute(position_init_, position_);

				auto compute_diff_coord = [&] (CMap2::Vertex v)
				{
					VEC3 centroid;
					centroid.setZero();
					uint32 nbav = 0;
					map2->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av)
					{
						centroid += position_[av];
						++nbav;
					});
					centroid /= SCALAR(nbav);
					diff_coord_[v] = centroid - position_[v];
				};

				map2->foreach_cell(compute_diff_coord);
//				free_vertex_set_->foreach_cell(compute_diff_coord);
//				handle_vertex_set_->foreach_cell(compute_diff_coord);

				nb_vertices_ = 0;
//				map2->foreach_cell([&] (CMap2::Vertex v)
//				{
//					v_index_[v] = nb_vertices_++;
//				});
				free_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					map_->get_map()->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
					{
						if (!free_vertex_set_->is_selected(av) && !handle_vertex_set_->is_selected(av))
						{
							free_boundary_mark_->mark(v);
							return false;
						}
						return true;
					});
					v_index_[v] = nb_vertices_++;
				});
				handle_vertex_set_->foreach_cell([&] (CMap2::Vertex v)
				{
					map_->get_map()->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
					{
						if (!free_vertex_set_->is_selected(av) && !handle_vertex_set_->is_selected(av))
						{
							std::cout << "handle vertex should not be on the boundary of the free zone" << std::endl;
							return false;
						}
						return true;
					});
					v_index_[v] = nb_vertices_++;
				});

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
	std::unique_ptr<CMap2::CellMarker<CMap2::Vertex::ORBIT>> free_boundary_mark_;

	CMap2::VertexAttribute<VEC3> position_;

	CMap2Handler::VertexSet* free_vertex_set_;
	CMap2Handler::VertexSet* handle_vertex_set_;

	bool initialized_;

	CMap2::VertexAttribute<VEC3> position_init_;
	CMap2::VertexAttribute<VEC3> diff_coord_;
	CMap2::VertexAttribute<MAT33> vertex_rotation_matrix_;
	CMap2::VertexAttribute<VEC3> rotated_diff_coord_;

	CMap2::VertexAttribute<uint32> v_index_;
	uint32 nb_vertices_;
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_MAP_PARAMETERS_H_
