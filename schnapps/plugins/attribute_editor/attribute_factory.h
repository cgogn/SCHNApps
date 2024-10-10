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

#include <schnapps/plugins/attribute_editor/plugin_attribute_editor_export.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <cgogn/core/cmap/attribute.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <QStringList>

namespace schnapps
{

namespace plugin_attribute_editor
{

namespace details
{

template<typename T, typename MapHandler>
cgogn::Attribute_T<T> create_attribute(MapHandler* mh, cgogn::Orbit ct, const std::string& attribute_name);

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap0Handler* mh, cgogn::Orbit ct, const std::string& attribute_name);
template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap1Handler* mh, cgogn::Orbit ct, const std::string& attribute_name);
template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap2Handler* mh, cgogn::Orbit ct, const std::string& attribute_name);
template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap3Handler* mh, cgogn::Orbit ct, const std::string& attribute_name);
template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::UndirectedGraphHandler* mh, cgogn::Orbit ct, const std::string& attribute_name);

}  // namespace details

template<typename MAP_TYPE>
class AttributeFactory final
{
public:
	using Self = AttributeFactory<MAP_TYPE>;
	using AttributeGen = cgogn::AttributeGen;
	template <typename T>
	using Attribute_T =  cgogn::Attribute_T<T>;
	template <typename T, cgogn::Orbit ORBIT>
	using Attribute =  cgogn::Attribute<T, ORBIT>;
	using MapHandler =
	typename std::conditional<std::is_same<MAP_TYPE, CMap0>::value, plugin_cmap_provider::CMap0Handler,
	typename std::conditional<std::is_same<MAP_TYPE, CMap1>::value, plugin_cmap_provider::CMap1Handler,
	typename std::conditional<std::is_same<MAP_TYPE, CMap2>::value, plugin_cmap_provider::CMap2Handler,
	typename std::conditional<std::is_same<MAP_TYPE, CMap3>::value, plugin_cmap_provider::CMap3Handler,
	typename std::conditional<std::is_same<MAP_TYPE, UndirectedGraph>::value, plugin_cmap_provider::UndirectedGraphHandler, std::false_type>::type>::type>::type>::type>::type;

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(AttributeFactory);

	inline void create_attribute(MapHandler* mh, const std::string& type_name, cgogn::Orbit ct, const std::string& attribute_name, const QStringList& default_value)
	{
		auto it = type_create_fn_map.find(type_name);
		if (it != type_create_fn_map.end())
			(it->second)(mh, ct, attribute_name, default_value);
		else
			cgogn_log_warning("AttributeFactory") << "Unable to create an attribute of type \"" << type_name << "\".";
	}

	inline static Self& get_instance()
	{
		static Self instance;
		return instance;
	}

	template<typename T>
	inline void register_type()
	{
		type_create_fn_map[cgogn::name_of_type(T())] = [&](MapHandler* mh, cgogn::Orbit ct, const std::string& attribute_name, const QStringList& default_value)
		{
			cgogn::Attribute_T<T> new_att = details::create_attribute<T>(mh, ct, attribute_name);
			if (new_att.is_valid() && !default_value.empty())
			{
				std::stringstream sstream;
				for (int i = 0; i < default_value.size() -1; ++i)
					sstream << default_value[i].toStdString() << ' ';
				bool extract_ok = static_cast<bool>(sstream << default_value[default_value.size() -1].toStdString());
				if (extract_ok)
				{
					T val;
					cgogn::serialization::parse(sstream, val);
					new_att.set_all_values(val);
				} else
					cgogn_log_warning("AttributeFactory") << "Unable to default initialize the attribute \"" << attribute_name << "\"";
			}
		};
	}

	const std::unordered_map<std::string, std::function<void(MapHandler*, cgogn::Orbit, const std::string&)>>& get_map() const
	{
		return type_create_fn_map;
	}

private:
	inline AttributeFactory()
	{
		register_type<cgogn::Dart>();
		register_type<bool>();
		register_type<int8>();
		register_type<int16>();
		register_type<int32>();
		register_type<int64>();
		register_type<uint8>();
		register_type<uint16>();
		register_type<uint32>();
		register_type<uint64>();
		register_type<float32>();
		register_type<float64>();
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

	std::unordered_map<std::string, std::function<void(MapHandler*, cgogn::Orbit, const std::string&, const QStringList&)>> type_create_fn_map;
};


namespace details
{

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap0Handler* mh, cgogn::Orbit ct, const std::string& attribute_name)
{
	switch(ct)
	{
		case cgogn::Orbit::DART:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::DART>>(attribute_name);
		default:
			return cgogn::Attribute_T<T>();
	}
}

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap1Handler* mh, cgogn::Orbit ct, const std::string& attribute_name)
{
	switch(ct)
	{
		case cgogn::Orbit::DART:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::DART>>(attribute_name);
		case cgogn::Orbit::PHI1:
		default:
			return cgogn::Attribute_T<T>();
	}
}

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap2Handler* mh, cgogn::Orbit ct, const std::string& attribute_name)
{
	switch(ct)
	{
		case cgogn::Orbit::DART:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::DART>>(attribute_name);
		case cgogn::Orbit::PHI1:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1>>(attribute_name);
		case cgogn::Orbit::PHI2:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI2>>(attribute_name);
		case cgogn::Orbit::PHI21:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI21>>(attribute_name);
		case cgogn::Orbit::PHI1_PHI2:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1_PHI2>>(attribute_name);
		default:
			return cgogn::Attribute_T<T>();
	}
}

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::CMap3Handler* mh, cgogn::Orbit ct, const std::string& attribute_name)
{
	switch(ct)
	{
		case cgogn::Orbit::DART:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::DART>>(attribute_name);
		case cgogn::Orbit::PHI1:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1>>(attribute_name);
		case cgogn::Orbit::PHI2:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI2>>(attribute_name);
		case cgogn::Orbit::PHI21:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI21>>(attribute_name);
		case cgogn::Orbit::PHI1_PHI2:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1_PHI2>>(attribute_name);
		case cgogn::Orbit::PHI1_PHI3:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1_PHI3>>(attribute_name);
		case cgogn::Orbit::PHI2_PHI3:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI2_PHI3>>(attribute_name);
		case cgogn::Orbit::PHI21_PHI31:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI21_PHI31>>(attribute_name);
		case cgogn::Orbit::PHI1_PHI2_PHI3:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1_PHI2_PHI3>>(attribute_name);
		default:
			return cgogn::Attribute_T<T>();
	}
}

template<typename T>
cgogn::Attribute_T<T> create_attribute(plugin_cmap_provider::UndirectedGraphHandler* mh, cgogn::Orbit ct, const std::string& attribute_name)
{
	switch(ct)
	{
		case cgogn::Orbit::DART:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::DART>>(attribute_name);
		case cgogn::Orbit::PHI1:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI1>>(attribute_name);
		case cgogn::Orbit::PHI2:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI2>>(attribute_name);
		case cgogn::Orbit::PHI21:
			return mh->map()->add_attribute<T, cgogn::Cell<cgogn::Orbit::PHI21>>(attribute_name);
		default:
			return cgogn::Attribute_T<T>();
	}
}

} // namespace details

} // namespace plugin_attribute_editor

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_FACTORY_H_
