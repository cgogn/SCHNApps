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


namespace schnapps
{

namespace plugin_merge
{

Plugin_Merge::Plugin_Merge() :
	merge_dialog_(nullptr),
	merge_action_(nullptr),
	plugin_cmap_provider_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_Merge::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_Merge::enable()
{
	merge_dialog_ = new MergeDialog(schnapps_, this);

	merge_action_ = schnapps_->add_menu_action("Merge;Merge Meshes", "merge meshes");
	connect(merge_action_, SIGNAL(triggered()), this, SLOT(merge_dialog()));
	connect(merge_dialog_->buttonBox,SIGNAL(accepted()), this, SLOT(merge_validated()));

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));

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

bool Plugin_Merge::merge(plugin_cmap_provider::CMap2Handler* first_map, const plugin_cmap_provider::CMap2Handler* second_map)
{
	CMap2::DartMarker dm(*first_map->map());
	return first_map->map()->merge(*second_map->map(), dm);
}

bool Plugin_Merge::merge(plugin_cmap_provider::CMap3Handler* first_map, const plugin_cmap_provider::CMap3Handler* second_map)
{
	CMap3::DartMarker dm(*first_map->map());
	return first_map->map()->merge(*second_map->map(), dm);
}

void Plugin_Merge::merge_validated()
{
	plugin_cmap_provider::CMap2Handler* mha = plugin_cmap_provider_->cmap2(merge_dialog_->combo_mapSelection->currentText());
	plugin_cmap_provider::CMap2Handler* mhb = plugin_cmap_provider_->cmap2(merge_dialog_->combo_mapSelection_2->currentText());

	if (!mha || !mhb)
	{
		plugin_cmap_provider::CMap3Handler* mh3a = plugin_cmap_provider_->cmap3(merge_dialog_->combo_mapSelection->currentText());
		plugin_cmap_provider::CMap3Handler* mh3b = plugin_cmap_provider_->cmap3(merge_dialog_->combo_mapSelection_2->currentText());
		if (mh3a && mh3b)
			merge(mh3a, mh3b);
	} else
		merge(mha, mhb);
}

} // namespace plugin_merge

} // namespace schnapps
