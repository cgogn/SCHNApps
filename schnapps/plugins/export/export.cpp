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

#include <schnapps/core/schnapps.h>
#include <schnapps/plugins/export/export.h>
#include <schnapps/plugins/export/export_dialog.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>

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
	plugin_cmap_provider_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_Export::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_Export::enable()
{
	delete export_dialog_;
	plugin_cmap_provider_ = reinterpret_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));

	if (!plugin_cmap_provider_)
		return false;

	export_dialog_ = new ExportDialog(schnapps_, this);

	export_mesh_action_ = schnapps_->add_menu_action("Export;Export Mesh", "export surface/volume mesh");
	connect(export_mesh_action_, SIGNAL(triggered()), this, SLOT(export_mesh_from_file_dialog()));

	return true;
}

void Plugin_Export::disable()
{
	disconnect(export_mesh_action_, SIGNAL(triggered()), this, SLOT(export_mesh_from_file_dialog()));

	schnapps_->remove_menu_action(export_mesh_action_);

	delete export_dialog_;
	export_dialog_ = nullptr;
}

void Plugin_Export::export_mesh(Object* mhg, cgogn::io::ExportOptions export_params)
{
	if (mhg)
	{
		if (CMap2Handler* m2h = dynamic_cast<CMap2Handler*>(mhg))
		{
			CMap2& cmap2 = *m2h->map();
			cgogn::io::export_surface(cmap2, export_params);
		}
		else
		{
			if (CMap3Handler* m3h = dynamic_cast<CMap3Handler*>(mhg))
			{
				CMap3& cmap3 = *m3h->map();
				cgogn_log_debug("export_mesh") << export_params.position_attributes_.size();
				cgogn_log_debug("export_mesh2") << export_params.attributes_to_export_.size();

				cgogn::io::export_volume(cmap3, export_params);
			}
		}
	}
}

void Plugin_Export::export_mesh_from_file_dialog()
{
	export_dialog_->show();
}

} // namespace plugin_export

} // namespace schnapps
