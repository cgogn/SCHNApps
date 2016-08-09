/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Modelisation                                                   *
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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_OPERATION_H
#define SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_OPERATION_H

#include "dll.h"
#include <schnapps/core/types.h>

#include <cgogn/core/basic/dart.h>
#include <cgogn/core/utils/logger.h>

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>


namespace schnapps
{

class MapHandlerGen;

namespace plugin_volume_modelisation
{

class SCHNAPPS_PLUGIN_VOLUME_MODELISATION_API Operator final
{
public:
	using Dart = cgogn::Dart;
	using FuncReturnType = std::vector<Dart>;
	using FuncParamType = const std::vector<Dart>&;
	using FuncType = std::function<FuncReturnType(MapHandlerGen*, FuncParamType)>;

	inline Operator() CGOGN_NOEXCEPT :
		cell_type_(CellType::Unknown),
		func_()
	{}

	inline Operator(CellType ct, FuncType&& fn) :
		cell_type_(ct),
		func_(std::forward<FuncType>(fn))
	{}

	inline Operator(Operator&& op) CGOGN_NOEXCEPT:
		cell_type_(op.cell_type_),
		func_(std::move(op.func_))
	{}

	inline Operator& operator=(Operator&& op) CGOGN_NOEXCEPT
	{
		if (this != &op)
		{
			cell_type_ = op.cell_type_;
			func_ = std::move(op.func_);
		}
		return *this;
	}

	CellType cell_type_;
	FuncType func_;
};

class SCHNAPPS_PLUGIN_VOLUME_MODELISATION_API VolumeOperation final
{
public:
	using FuncType = Operator::FuncType;

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(VolumeOperation);
	inline VolumeOperation() {}

	void add_operation(const std::string& op_name, CellType ct, FuncType&& func);
	const Operator* get_operator(const std::string& op_name) const;

	std::vector<std::string> get_operations() const;

private:
	std::unordered_map<std::string, Operator> func_map_;
};

} // namespace plugin_volume_modelisation
} // namespace schnapps




#endif // SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_OPERATION_H
