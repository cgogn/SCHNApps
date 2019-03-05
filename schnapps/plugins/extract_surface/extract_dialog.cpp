/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin ExtractSurface                                                        *
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

#include <schnapps/plugins/extract_surface/extract_surface.h>
#include <schnapps/plugins/extract_surface/extract_dialog.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>
#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_extract_surface
{

using CMap3Handler = plugin_cmap_provider::CMap3Handler;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

ExtractDialog::ExtractDialog(SCHNApps* s, Plugin_ExtractSurface* p)
{
	schnapps_ = s;
	plugin_ = p;
	updating_ui_ = false;
	setupUi(this);

	schnapps_->foreach_object([&](Object* mhg)
	{
		map_added(mhg);
	});

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(map_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(map_removed(Object*)));
	connect(this->comboBoxMapSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(extract_validated()));
}

void ExtractDialog::map_added(Object* mhg)
{
	if (dynamic_cast<CMap3Handler*>(mhg))
		comboBoxMapSelection->addItem(mhg->name());
}

void ExtractDialog::map_removed(Object* mhg)
{
	if (dynamic_cast<CMap3Handler*>(mhg))
		comboBoxMapSelection->removeItem(this->comboBoxMapSelection->findText(mhg->name()));
	if (mhg->name() == comboBoxMapSelection->currentText())
		comboBoxPositionAttribute->clear();
}

void ExtractDialog::selected_map_changed(const QString& map_name)
{
	comboBoxPositionAttribute->clear();

	if (CMap3Handler* mhg = plugin_->map_provider()->cmap3(map_name))
	{
		const auto& vert_att_cont = mhg->map()->attribute_container(CMap3::Vertex::ORBIT);
		for (const auto& att_name : vert_att_cont.names())
		{
			this->comboBoxPositionAttribute->addItem(QString::fromStdString(att_name));
		}
	}
}

void ExtractDialog::extract_validated()
{
	if (CMap3Handler* handler_map3 = plugin_->map_provider()->cmap3(comboBoxMapSelection->currentText()))
	{
		CMap2Handler* handler_map2 = plugin_->map_provider()->add_cmap2(handler_map3->name() + "_surface");
		plugin_->extract_surface(handler_map3, handler_map2, "position");
	}
}

} // namespace plugin_extract_surface

} // namespace schnapps
