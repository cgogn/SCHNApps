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

#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

uint32 CMapCellsSetGen::cells_set_count_ = 0;

CMapCellsSetGen::CMapCellsSetGen(const QString& name) :
	name_(name),
	mutually_exclusive_(false),
	selection_changed_(false)
{
	++cells_set_count_;
}

CMapCellsSetGen::~CMapCellsSetGen()
{}

} // namespace plugin_cmap_provider

} // namespace schnapps
