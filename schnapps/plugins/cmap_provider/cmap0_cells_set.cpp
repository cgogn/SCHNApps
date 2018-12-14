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

#include <schnapps/plugins/cmap_provider/cmap0_cells_set.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

uint32 CMap0CellsSetGen::cells_set_count_ = 0;

CMap0CellsSetGen::CMap0CellsSetGen(const CMap0Handler& mh, const QString& name) :
	mh_(mh),
	name_(name),
	mutually_exclusive_(false),
	selection_changed_(false)
{
	++cells_set_count_;
}

CMap0CellsSetGen::~CMap0CellsSetGen()
{}

} // namespace plugin_cmap_provider

} // namespace schnapps
