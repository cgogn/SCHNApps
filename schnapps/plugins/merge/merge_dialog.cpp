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

#include <dll.h>
#include <merge_dialog.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/schnapps.h>
#include <merge_plugin.h>

namespace schnapps
{

namespace merge_plugin
{

MergeDialog::MergeDialog(SCHNApps* s, MergePlugin* p) :
	schnapps_(s),
	plugin_(p)
{
	setupUi(this);

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(merge_validated()));

	schnapps_->foreach_map([&] (MapHandlerGen* mhg)
	{
		map_added(mhg);
	});
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void MergeDialog::merge_validated()
{
	MapHandlerGen* mhg1 = schnapps_->get_map(combo_mapSelection->currentText());
	MapHandlerGen* mhg2 = schnapps_->get_map(combo_mapSelection_2->currentText());

	if (!mhg1 || !mhg2)
		return;

	MapHandlerGen* copy_mhg1 = schnapps_->duplicate_map(mhg1->get_name());
	plugin_->merge(copy_mhg1, mhg2);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void MergeDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
	{
		combo_mapSelection->addItem(mhg->get_name());
		combo_mapSelection_2->addItem(mhg->get_name());
	}
}

void MergeDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
	{
		combo_mapSelection->removeItem(combo_mapSelection->findText(mhg->get_name()));
		combo_mapSelection_2->removeItem(combo_mapSelection_2->findText(mhg->get_name()));
	}
}

} // namespace merge_plugin

} // namespace schnapps
