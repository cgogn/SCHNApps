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
			linked_ = true;
		}
		else
		{
			linked_ = false;
		}
		return linked_;
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
