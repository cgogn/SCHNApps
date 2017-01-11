/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Merge plugin                                                                 *
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

#include <merge_plugin.h>
#include <merge_dialog.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace merge_plugin
{

MergePlugin::MergePlugin() :
	merge_action_(nullptr),
	merge_dialog_(nullptr)
{}

MergePlugin::~MergePlugin()
{
	delete merge_dialog_;
}

bool MergePlugin::merge(MapHandlerGen* first_map, const MapHandlerGen* second_map)
{
	return first_map->merge(second_map);
}

bool MergePlugin::enable()
{
	merge_action_ = schnapps_->add_menu_action("Merge;Merge Meshes", "merge meshes");
	connect(merge_action_, SIGNAL(triggered()), this, SLOT(merge_dialog()));

	if (!merge_dialog_)
		merge_dialog_ = new MergeDialog(schnapps_, this);

	return true;
}

void MergePlugin::disable()
{
	schnapps_->remove_menu_action(merge_action_);
}

void MergePlugin::merge_dialog()
{
	merge_dialog_->show();
}

} // namespace merge_plugin
} // namespace schnapps
