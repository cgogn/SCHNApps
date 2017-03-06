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

#ifndef SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_
#define SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_

#include "dll.h"
#include <ui_export_dialog.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_export
{

class Plugin_Export;

class SCHNAPPS_PLUGIN_EXPORT_API ExportDialog : public QDialog, public Ui::MapExport
{
	Q_OBJECT
	friend class Plugin_Export;

public:
	ExportDialog(SCHNApps* s, Plugin_Export* p);

private slots:
	void selected_map_changed(QString map_name);
	void position_att_changed(QString pos_name);
	void output_file_changed(QString output);
	void map_added(MapHandlerGen*);
	void map_removed(MapHandlerGen*);
	void choose_file();
	void binary_option_changed(bool b);
	void compress_option_changed(bool b);
	void reinit();
	void export_validated();
	void vertex_attribute_changed(QListWidgetItem* item);
	void cell_attribute_changed(QListWidgetItem* item);

private:
	SCHNApps* schnapps_;
	Plugin_Export* plugin_;
	bool updating_ui_;
};

} // namespace plugin_export
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_
