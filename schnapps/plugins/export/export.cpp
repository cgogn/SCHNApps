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

#include <export.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <export_dialog.h>
#include <cgogn/io/map_import.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QAction>

namespace schnapps
{

ExportParams::ExportParams() :
	map_name_(),
	position_attribute_name_(),
	output_(),
	binary_(false),
	compress_(false)
{}


Plugin_Export::Plugin_Export() {
	export_dialog_ = nullptr;
}

Plugin_Export::~Plugin_Export() {
	delete export_dialog_;
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

void Plugin_Export::export_mesh(const QString& filename)
{
	MapHandlerGen* mhg = schnapps_->get_selected_map();
	if(!mhg)
		return;

}

void Plugin_Export::export_mesh_from_file_dialog()
{
//	QString filename = QFileDialog::getSaveFileName(nullptr, "Export mesh", schnapps_->get_app_path(), "Surface Mesh Files (*.ply *.off *.stl *.vtk *.vtp *.obj); Surface Mesh Files (*.vtk *.vtu *.tet *.nas)");
//	export_mesh(filename);
//	QInputDialog
	export_dialog_->show();
}




Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")


} // namespace schnapps
