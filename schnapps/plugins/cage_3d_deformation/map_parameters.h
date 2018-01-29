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

#ifndef SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_MAP_PARAMETERS_H_
#define SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_MAP_PARAMETERS_H_

#include <schnapps/plugins/cage_3d_deformation/dll.h>

#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/area.h>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

class Plugin_Cage3dDeformation;

struct MapParameters
{
	friend class Plugin_Cage3dDeformation;

	MapParameters() :
		control_map_(nullptr),
		deformed_map_(nullptr),
		linked_(false)
	{}

	~MapParameters()
	{}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	CMap2Handler* get_control_map() const { return control_map_; }
	const CMap2Handler::VertexAttribute<VEC3>& get_control_position_attribute() const { return control_position_; }
	QString get_control_position_attribute_name() const { return QString::fromStdString(control_position_.name()); }

	MapHandlerGen* get_deformed_map() const { return deformed_map_; }
	const MapHandlerGen::Attribute_T<VEC3>& get_deformed_position_attribute() const { return deformed_position_; }
	QString get_deformed_position_attribute_name() const { return QString::fromStdString(deformed_position_.name()); }

	bool get_linked() const { return linked_; }

	bool toggle_control()
	{
		if (!linked_ &&
			control_map_ &&
			control_position_.is_valid() &&
			deformed_map_ &&
			deformed_position_.is_valid()
		)
		{
			if (deformed_map_->dimension() == 2)
			{
				CMap2Handler* dmh2 = static_cast<CMap2Handler*>(deformed_map_);
				coord_ = dmh2->get_attribute<std::vector<double>, CMap2Handler::Vertex::ORBIT>("__mvc_coord");
				if (!coord_.is_valid())
					coord_ = dmh2->add_attribute<std::vector<double>, CMap2Handler::Vertex::ORBIT>("__mvc_coord");

				CMap2* cm2 = control_map_->get_map();
				uint32 nbcv = cm2->nb_cells<CMap2::Vertex>();
				CMap2* dm2 = dmh2->get_map();
				dm2->foreach_cell([&] (CMap2::Vertex dv)
				{
					const VEC3& dv_pos = deformed_position_[dv.dart];
					std::vector<double>& dv_coords = coord_[dv.dart];
					dv_coords.clear();
					dv_coords.reserve(nbcv);
					double coords_sum = 0.;
					cm2->foreach_cell([&] (CMap2::Vertex cv)
					{
						const VEC3& cv_pos = control_position_[cv];
						VEC3 e = cv_pos - dv_pos;
						double r = e.norm();
						e.normalize();
						double sum = 0.;
						cm2->foreach_incident_face(cv, [&] (CMap2::Face cvif)
						{
							VEC3 prev_pos = control_position_[cm2->phi_1(cvif.dart)];
							VEC3 next_pos = control_position_[cm2->phi1(cvif.dart)];

							VEC3 prev_e = (prev_pos - dv_pos).normalized();
							VEC3 next_e = (next_pos - dv_pos).normalized();

							VEC3 n_pv = prev_e.cross(e).normalized();
							VEC3 n_vn = e.cross(next_e).normalized();
							VEC3 n_np = next_e.cross(prev_e).normalized();

							double a_pv = cgogn::geometry::angle(prev_e, e);
							double a_vn = cgogn::geometry::angle(e, next_e);
							double a_np = cgogn::geometry::angle(next_e, prev_e);

							double u = ( a_np + ((a_vn * n_vn).dot(n_np)) + ((a_pv * n_pv).dot(n_np)) ) / ((2.0*e).dot(n_np));

							sum += u;
						});
						double w = sum / r;
						coords_sum += w;
						dv_coords.push_back(w);
					});
					for (auto& dvc : dv_coords)
						dvc /= coords_sum;
				});
			}
			else // deformed_map_->dimension() == 3
			{
				CMap3Handler* dm3 = static_cast<CMap3Handler*>(deformed_map_);
				coord_ = dm3->get_attribute<std::vector<double>, CMap3Handler::Vertex::ORBIT>("__mvc_coord");
				if (!coord_.is_valid())
					coord_ = dm3->add_attribute<std::vector<double>, CMap3Handler::Vertex::ORBIT>("__mvc_coord");
			}

			linked_ = true;
		}
		else
		{
			linked_ = false;
		}
		return linked_;
	}

	void update_deformed_map()
	{
		if (linked_)
		{
			if (deformed_map_->dimension() == 2)
			{
				CMap2Handler* dmh2 = static_cast<CMap2Handler*>(deformed_map_);
				CMap2* cm2 = control_map_->get_map();
				CMap2* dm2 = dmh2->get_map();
				dm2->foreach_cell([&] (CMap2::Vertex dv)
				{
					VEC3 newpos(0,0,0);

					std::vector<double>& dv_coords = coord_[dv.dart];
					uint32 cvidx = 0;
					cm2->foreach_cell([&] (CMap2::Vertex cv)
					{
						const VEC3& cv_pos = control_position_[cv];
						newpos += dv_coords[cvidx] * cv_pos;
						++cvidx;
					});

					deformed_position_[dv.dart] = newpos;
				});

				deformed_map_->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(deformed_position_.name()));
			}
			else // deformed_map_->dimension() == 3
			{

			}
		}
	}

private:

	bool set_control_position_attribute(const QString& attribute_name)
	{
		control_position_ = control_map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name);
		if (control_position_.is_valid())
			return true;
		else
			return false;
	}

	bool set_deformed_map(MapHandlerGen* map)
	{
		deformed_map_ = map;
		return true;
	}

	bool set_deformed_position_attribute(const QString& attribute_name)
	{
		if (deformed_map_->dimension() == 2)
			deformed_position_ = static_cast<CMap2Handler*>(deformed_map_)->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name);
		else // deformed_map_->dimension() == 3
			deformed_position_ = static_cast<CMap3Handler*>(deformed_map_)->get_attribute<VEC3, CMap3::Vertex::ORBIT>(attribute_name);
		if (deformed_position_.is_valid())
			return true;
		else
			return false;
	}

	CMap2Handler* control_map_;
	CMap2Handler::VertexAttribute<VEC3> control_position_;

	MapHandlerGen* deformed_map_;
	MapHandlerGen::Attribute_T<VEC3> deformed_position_;
	MapHandlerGen::Attribute_T<std::vector<double>> coord_;

	bool linked_;
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_MAP_PARAMETERS_H_
