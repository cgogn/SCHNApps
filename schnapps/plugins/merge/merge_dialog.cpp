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

#include <schnapps/plugins/merge/dll.h>
#include <schnapps/plugins/merge/merge.h>
#include <schnapps/plugins/merge/merge_dialog.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>
#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_merge
{

MergeDialog::MergeDialog(SCHNApps* s, Plugin_Merge* p) :
	schnapps_(s),
	plugin_(p)
{
	setupUi(this);
}

void MergeDialog::update_map_list()
{
	combo_mapSelection->clear();
	combo_mapSelection_2->clear();
	schnapps_->foreach_object([this] (Object* o)
	{
		plugin_cmap_provider::CMap2Handler* mh2 = dynamic_cast<plugin_cmap_provider::CMap2Handler*>(o);
		if (mh2) {
			combo_mapSelection->addItem(mh2->name());
			combo_mapSelection_2->addItem(mh2->name());
		} else {
			plugin_cmap_provider::CMap3Handler* mh3 = dynamic_cast<plugin_cmap_provider::CMap3Handler*>(o);
			if (mh3) {
				combo_mapSelection->addItem(mh3->name());
				combo_mapSelection_2->addItem(mh3->name());
			}
		}
	});
}

void MergeDialog::showEvent(QShowEvent* e)
{
	update_map_list();
	QDialog::showEvent(e);
}


} // namespace plugin_merge

} // namespace schnapps
