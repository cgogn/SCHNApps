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

#ifndef SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_
#define SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_

#include "dll.h"
#include <ui_export_dialog.h>

#include <cgogn/io/io_utils.h>

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

public:

	ExportDialog(SCHNApps* s, Plugin_Export* p);

private slots:

	void selected_map_changed(const QString& map_name);
	void position_attribute_changed(const QString& pos_name);
	void choose_file();

	void map_added(MapHandlerGen*);
	void map_removed(MapHandlerGen*);

	void export_validated();

private:

	SCHNApps* schnapps_;
	Plugin_Export* plugin_;

	MapHandlerGen* selected_map_;
};

} // namespace plugin_export

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_
