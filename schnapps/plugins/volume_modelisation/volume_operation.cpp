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

#include <schnapps/plugins/volume_modelisation/volume_operation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/core/cmap/cmap3.h>
#include <cgogn/modeling/algos/tetrahedralization.h>

namespace schnapps
{

namespace plugin_volume_modelisation
{

void VolumeOperation::add_operation(const std::string& op_name, CellType ct, FuncType&& func)
{
	auto func_it = func_map_.find(op_name);
	if (func_it != func_map_.end())
	{
		cgogn_log_debug("VolumeOperation::add_operation") << "operation \"" << op_name << "\" already registered.";
		return;
	}
	func_map_[op_name] = MapOperator(ct, std::move(func));
}

const MapOperator* VolumeOperation::get_operator(const std::string& op_name) const
{
	auto func_it = func_map_.find(op_name);
	if (func_it == func_map_.end())
		return nullptr;
	else
		return &(func_it->second);
}

std::vector<std::string> VolumeOperation::get_operations() const
{
	std::vector<std::string> res;
	res.reserve(func_map_.size());

	for (const auto& it : func_map_)
		res.push_back(it.first);

	return res;
}


} // namespace plugin_volume_modelisation
} // namespace schnapps
