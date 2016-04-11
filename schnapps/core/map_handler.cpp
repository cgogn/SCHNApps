/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
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

#include <schnapps/core/map_handler.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

MapHandlerGen::MapHandlerGen(const QString& name, SCHNApps* schnapps, MapBaseData* map) :
	name_(name),
	schnapps_(schnapps),
	map_(map)
{}

MapHandlerGen::~MapHandlerGen()
{}

bool MapHandlerGen::is_selected_map() const
{
	return schnapps_->get_selected_map() == this;
}

void MapHandlerGen::link_view(View* view)
{
	if (view && !views_.contains(view))
		views_.push_back(view);
}

void MapHandlerGen::unlink_view(View* view)
{
	views_.removeOne(view);
}

} // namespace schnapps
