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

#include <dll.h>
#include <extract_dialog.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/schnapps.h>
#include <extract_surface.h>

namespace schnapps
{

namespace plugin_extract_surface
{

ExtractDialog::ExtractDialog(SCHNApps* s, Plugin_ExtractSurface* p)
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
	connect(this->comboBoxMapSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(extract_validated()));
}

void ExtractDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg && mhg->dimension() == 3u)
		comboBoxMapSelection->addItem(mhg->get_name());
}

void ExtractDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg && mhg->dimension() == 3u)
		comboBoxMapSelection->removeItem(this->comboBoxMapSelection->findText(mhg->get_name()));
	if (mhg->get_name() == comboBoxMapSelection->currentText())
		comboBoxPositionAttribute->clear();
}

void ExtractDialog::selected_map_changed(const QString& map_name)
{
	comboBoxPositionAttribute->clear();

	if (MapHandlerGen* mhg = schnapps_->get_map(map_name))
	{
		const auto* vert_att_cont = mhg->attribute_container(CellType::Vertex_Cell);
		for (const auto& att_name : vert_att_cont->names())
		{
			this->comboBoxPositionAttribute->addItem(QString::fromStdString(att_name));
		}
	}
}

void ExtractDialog::extract_validated()
{
	MapHandlerGen* mhg = schnapps_->get_map(comboBoxMapSelection->currentText());
	if (mhg)
	{
		MapHandlerGen* handler_map2 = schnapps_->add_map(mhg->get_name() + "_surface", 2);
		plugin_->extract_surface(mhg, handler_map2, "position");
	}
}

} // namespace plugin_extract_surface
} // namespace schnapps
