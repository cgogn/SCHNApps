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

MergeDialog::MergeDialog(SCHNApps* s, MergePlugin* p)
{
	schnapps_ = s;
	plugin_ = p;
	updating_ui_ = false;
	setupUi(this);

	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		map_added(mhg);
	});

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(merge_validated()));
}

void MergeDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
	{
		comboBoxMapSelection->addItem(mhg->get_name());
		comboBoxMapSelection_2->addItem(mhg->get_name());
	}
}

void MergeDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
	{
		comboBoxMapSelection->removeItem(this->comboBoxMapSelection->findText(mhg->get_name()));
		comboBoxMapSelection_2->removeItem(this->comboBoxMapSelection->findText(mhg->get_name()));
	}
}


void MergeDialog::merge_validated()
{
	MapHandlerGen* mhg1 = schnapps_->get_map(comboBoxMapSelection->currentText());
	MapHandlerGen* mhg2 = schnapps_->get_map(comboBoxMapSelection_2->currentText());

	if (!mhg1 || !mhg2)
		return;

	MapHandlerGen* copy_mhg1 = schnapps_->duplicate_map(mhg1->get_name());
	plugin_->merge(copy_mhg1, mhg2);
}

} // namespace merge_plugin
} // namespace schnapps
