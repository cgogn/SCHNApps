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

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/types.h>

#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/area.h>

#include <QString>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

class Plugin_Cage3dDeformation;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

struct MapParameters
{
	friend class Plugin_Cage3dDeformation;

	MapParameters() :
		deformed_map_(nullptr),
		control_map_(nullptr),
		linked_(false),
		updating_(false)
	{}

	~MapParameters()
	{}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	CMap2Handler* deformed_map() const { return deformed_map_; }
	const CMap2::VertexAttribute<VEC3>& deformed_position_attribute() const { return deformed_position_; }

	CMap2Handler* control_map() const { return control_map_; }
	const CMap2::VertexAttribute<VEC3>& control_position_attribute() const { return control_position_; }

	bool linked() const { return linked_; }
	bool updating() const { return updating_; }

private:

	bool set_deformed_position_attribute(const QString& attribute_name)
	{
		deformed_position_ = deformed_map_->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
		return deformed_position_.is_valid();
	}

	bool set_control_map(CMap2Handler* mh)
	{
		control_map_ = mh;
		return true;
	}

	bool set_control_position_attribute(const QString& attribute_name)
	{
		control_position_ = control_map_->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
		return control_position_.is_valid();
	}

public:

	bool toggle_control()
	{
		if (!linked_ &&
			deformed_map_ &&
			deformed_position_.is_valid() &&
			control_map_ &&
			control_position_.is_valid()
		)
		{
			CMap2* cm2 = control_map_->map();
			uint32 nbcv = cm2->nb_cells<CMap2::Vertex>();
			uint32 nbdv = deformed_map_->map()->nb_cells<CMap2::Vertex>();

			deformed_position_saved_ = deformed_map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("__deformed_position_saved");

			coords_.setZero();
			coords_.resize(nbdv, nbcv);

			cage_pos_.setZero();
			cage_pos_.resize(nbcv, 3);

			def_pos_.setZero();
			def_pos_.resize(nbdv, 3);

			std::vector<double> coords_tmp(nbcv);

			uint32 dvidx = 0;
			deformed_map_->map()->foreach_cell([&] (CMap2::Vertex v)
			{
				const VEC3& dv_pos = deformed_position_[v];

				for (auto& c : coords_tmp) c = 0.;

				double coords_sum = 0.;

				uint32 cvidx = 0;
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

					coords_tmp[cvidx] = w;

					++cvidx;
				});

				cvidx = 0;
				for (auto& c : coords_tmp)
				{
					c /= coords_sum;
					coords_(dvidx, cvidx) = c;
					++cvidx;
				}

				deformed_position_saved_[v] = dv_pos;

				++dvidx;
			});

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
			updating_ = true;

			CMap2* cm2 = control_map_->map();

			uint32 cvidx = 0;
			cm2->foreach_cell([&] (CMap2::Vertex cv)
			{
				const VEC3& cvp = control_position_[cv];
				cage_pos_(cvidx, 0) = cvp[0];
				cage_pos_(cvidx, 1) = cvp[1];
				cage_pos_(cvidx, 2) = cvp[2];
				++cvidx;
			});

			def_pos_ = coords_ * cage_pos_;

			uint32 dvidx = 0;
			deformed_map_->map()->foreach_cell([&] (CMap2::Vertex v)
			{
				VEC3& def = deformed_position_[v];
				def[0] = def_pos_(dvidx, 0);
				def[1] = def_pos_(dvidx, 1);
				def[2] = def_pos_(dvidx, 2);
				deformed_position_saved_[v] = def;
				++dvidx;
			});

			deformed_map_->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(deformed_position_.name()));

			updating_ = false;
		}
	}

	void update_control_map()
	{
		if (linked_)
		{
			updating_ = true;

			CMap2* cm2 = control_map_->map();
			uint32 nbcv = cm2->nb_cells<CMap2::Vertex>();
			uint32 nbdv = deformed_map_->map()->nb_cells<CMap2::Vertex>();

			Eigen::VectorXd rhs(nbdv);

			for (uint32 dim = 0; dim < 3; ++dim)
			{
				uint32 dvidx = 0;
				deformed_map_->map()->foreach_cell([&] (CMap2::Vertex v)
				{
					rhs[dvidx] = 2.0 * (deformed_position_[v] - deformed_position_saved_[v])[dim];
					++dvidx;
				});

				Eigen::VectorXd L = (coords_ * coords_.transpose()).ldlt().solve(rhs);

				Eigen::VectorXd displ = 0.5 * (coords_.transpose() * L);

				uint32 cvidx = 0;
				cm2->foreach_cell([&] (CMap2::Vertex cv)
				{
					VEC3& cvp = control_position_[cv];
					cvp[dim] += displ[cvidx];
					++cvidx;
				});
			}

//			update_deformed_map();

			control_map_->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(control_position_.name()));

			updating_ = false;
		}
	}

private:

	CMap2Handler* deformed_map_;
	CMap2::VertexAttribute<VEC3> deformed_position_;
	CMap2::VertexAttribute<VEC3> deformed_position_saved_;

	CMap2Handler* control_map_;
	CMap2::VertexAttribute<VEC3> control_position_;

	Eigen::MatrixXd coords_;
	Eigen::MatrixXd cage_pos_;
	Eigen::MatrixXd def_pos_;

	bool linked_;
	bool updating_;
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_MAP_PARAMETERS_H_
