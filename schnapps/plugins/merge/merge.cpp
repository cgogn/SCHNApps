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

#include <schnapps/plugins/merge/merge.h>
#include <schnapps/plugins/merge/merge_dialog.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_merge
{

Plugin_Merge::Plugin_Merge() :
	merge_action_(nullptr),
	merge_dialog_(nullptr)
{}

bool Plugin_Merge::enable()
{
	merge_dialog_ = new MergeDialog(schnapps_, this);

	merge_action_ = schnapps_->add_menu_action("Merge;Merge Meshes", "merge meshes");
	connect(merge_action_, SIGNAL(triggered()), this, SLOT(merge_dialog()));

	return true;
}

void Plugin_Merge::disable()
{
	schnapps_->remove_menu_action(merge_action_);

	delete merge_dialog_;
}

void Plugin_Merge::schnapps_closing()
{
	merge_dialog_->close();
}

void Plugin_Merge::merge_dialog()
{
	merge_dialog_->show();
}

bool Plugin_Merge::merge(MapHandlerGen* first_map, const MapHandlerGen* second_map)
{
	return first_map->merge(second_map);
}

} // namespace plugin_merge

} // namespace schnapps
