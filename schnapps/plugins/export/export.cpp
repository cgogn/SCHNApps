/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Export                                                                *
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

#include <export.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <export_dialog.h>
#include <cgogn/io/map_export.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QAction>

namespace schnapps
{

namespace plugin_export
{

Plugin_Export::Plugin_Export() :
	export_mesh_action_(nullptr),
	export_dialog_(nullptr),
	export_params_(nullptr),
	map_name_()
{
	export_params_ = new cgogn::io::ExportOptions(cgogn::io::ExportOptions::create());
}

Plugin_Export::~Plugin_Export()
{
	delete export_params_;
	delete export_dialog_;
}

void Plugin_Export::export_mesh()
{
	MapHandlerGen* mhg = schnapps_->get_map(map_name_);
	if (mhg)
	{
		std::vector<std::pair<cgogn::Orbit, std::string>> other_attributes;
		if (CMap2Handler* m2h = dynamic_cast<CMap2Handler*>(mhg))
		{
			CMap2& cmap2 = *m2h->get_map();
			cgogn::io::export_surface(cmap2, *(this->export_params_));
		}
		else
		{
			if (CMap3Handler* m3h = dynamic_cast<CMap3Handler*>(mhg))
			{
				CMap3& cmap3 = *m3h->get_map();
				cgogn::io::export_volume(cmap3, *(this->export_params_));
			}
		}
	}
}

bool Plugin_Export::enable()
{
	export_mesh_action_ = schnapps_->add_menu_action("Export;Export Mesh", "export surface/volume mesh");
	connect(export_mesh_action_, SIGNAL(triggered()), this, SLOT(export_mesh_from_file_dialog()));

	if (!export_dialog_)
		export_dialog_ = new ExportDialog(schnapps_, this);

	return true;
}

void Plugin_Export::disable()
{
	schnapps_->remove_menu_action(export_mesh_action_);
}

void Plugin_Export::export_mesh(const QString& /*filename*/)
{
	MapHandlerGen* mhg = schnapps_->get_selected_map();
	if(!mhg)
		return;
	cgogn_log_warning("Plugin_Export::export_mesh()") << "TODO: Plugin_Export::export_mesh().";
}

void Plugin_Export::export_mesh_from_file_dialog()
{
	export_dialog_->show();
}

} // namespace plugin_export
} // namespace schnapps
