/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Attribute Editor                                                      *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#ifndef SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_FACTORY_H_
#define SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_FACTORY_H_

#include "dll.h"
#include <schnapps/core/map_handler.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace schnapps
{

namespace plugin_attribute_editor
{

template<typename MAP_TYPE>
class AttributeFactory final
{
public:
	using Self = AttributeFactory<MAP_TYPE>;
	using AttributeGen = MapHandlerGen::AttributeGen;
	template <typename T>
	using Attribute_T =  MapHandlerGen::Attribute_T<T>;
	template <typename T, cgogn::Orbit ORBIT>
	using Attribute =  MapHandlerGen::Attribute<T, ORBIT>;
	using MapHandler = schnapps::MapHandler<MAP_TYPE>;

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(AttributeFactory);

	inline void create_attribute(MapHandler* mh, const std::string& type_name, CellType ct, const std::string& attribute_name)
	{
		if (mh->dimension() != MAP_TYPE::DIMENSION)
			return;

		auto it = type_create_fn_map.find(type_name);
		if (it != type_create_fn_map.end())
			(it->second)(mh, ct, attribute_name);
	}

	inline static Self& get_instance()
	{
		static Self instance;
		return instance;
	}

	template<typename T>
	inline void register_type()
	{
		type_create_fn_map[cgogn::name_of_type(T())] = [&](MapHandler* mh, CellType ct, const std::string& attribute_name) { create_attribute<T>(mh, ct, attribute_name); };
	}

	const std::unordered_map<std::string, std::function<void(MapHandler*, CellType, const std::string&)>>& get_map() const
	{
		return type_create_fn_map;
	}

private:
	inline AttributeFactory()
	{
		register_type<bool>();
		register_type<int8>();
		register_type<int16>();
		register_type<int32>();
		register_type<int64>();
		register_type<uint8>();
		register_type<uint16>();
		register_type<uint32>();
		register_type<uint64>();
		register_type<VEC2F>();
		register_type<VEC2D>();
		register_type<VEC3F>();
		register_type<VEC3D>();
		register_type<VEC4F>();
		register_type<VEC4D>();
		register_type<MAT2F>();
		register_type<MAT2D>();
		register_type<MAT3F>();
		register_type<MAT3D>();
		register_type<MAT4F>();
		register_type<MAT4D>();
	}

	template<typename T>
	inline void create_attribute(MapHandler* mh, CellType ct, const std::string& attribute_name)
	{
		switch (ct) {
			case CellType::Dart_Cell:
					create_attribute<T, MAP_TYPE::CDart::ORBIT>(mh, attribute_name); break;
			case CellType::Vertex_Cell:
					create_attribute<T, MAP_TYPE::Vertex::ORBIT>(mh, attribute_name); break;
			case CellType::Edge_Cell:
					create_attribute<T, MAP_TYPE::Edge::ORBIT>(mh, attribute_name); break;
			case CellType::Face_Cell:
				create_attribute<T, MAP_TYPE::Face::ORBIT>(mh, attribute_name); break;
			case CellType::Volume_Cell:
					create_attribute<T, MAP_TYPE::Volume::ORBIT>(mh, attribute_name); break;
		}
	}

	template<typename T, cgogn::Orbit ORBIT>
	inline void create_attribute(MapHandler* mh, const std::string& attribute_name)
	{
		if (mh)
			mh->template add_attribute<T, ORBIT>(QString::fromStdString(attribute_name));
	}


	std::unordered_map<std::string, std::function<void(MapHandler*, CellType, const std::string&)>> type_create_fn_map;
};

} // namespace plugin_attribute_editor
} // namespace schnapps


#endif // SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_FACTORY_H_
