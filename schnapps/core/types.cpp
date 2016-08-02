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

#include "types.h"
#include <functional>
#include <unordered_map>

namespace schnapps
{

SCHNAPPS_CORE_API std::string cell_type_name(CellType ct)
{
	switch (ct)
	{
		case Dart_Cell: return "Dart"; break;
		case Vertex_Cell: return "Vertex"; break;
		case Edge_Cell: return "Edge"; break;
		case Face_Cell: return "Face"; break;
		case Volume_Cell: return "Volume"; break;
		case Unknown: return "UNKNOWN"; break;
	}
#ifdef NDEBUG
	return "UNKNOWN";  // little trick to  avoid warning on VS
#endif
}

SCHNAPPS_CORE_API CellType cell_type(const std::string& name)
{
	static const std::unordered_map<std::string, std::function<CellType()>> map =
	{
	{cell_type_name(CellType::Dart_Cell), [&]() { return CellType::Dart_Cell; }},
	{cell_type_name(CellType::Vertex_Cell), [&]() { return CellType::Vertex_Cell; }},
	{cell_type_name(CellType::Edge_Cell), [&]() { return CellType::Edge_Cell; }},
	{cell_type_name(CellType::Face_Cell), [&]() { return CellType::Face_Cell; }},
	{cell_type_name(CellType::Volume_Cell), [&]() { return CellType::Volume_Cell; }},
	{cell_type_name(CellType::Unknown), [&]() { return CellType::Unknown; }}
	};

	auto it = map.find(name);
	if (it != map.end())
		return (it->second)();
	else
		return CellType::Unknown;
}
} // namespace schnapps

