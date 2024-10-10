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

#include <schnapps/plugins/export/plugin_export_export.h>
#include <ui_export_dialog.h>

namespace cgogn
{
namespace io
{
class ExportOptions;
}
}

namespace schnapps
{

class SCHNApps;
class Object;

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap0Handler;
}

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
}

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap3Handler;
}

namespace plugin_export
{

class Plugin_Export;

class PLUGIN_EXPORT_EXPORT ExportDialog : public QDialog, public Ui::ExportDialog
{
	Q_OBJECT

public:

	ExportDialog(SCHNApps* s, Plugin_Export* p);

private slots:

	// slots called from UI signals
	void selected_map_changed(const QString& map_name);
	void position_attribute_changed(const QString& pos_name);
	void choose_file();
	void export_validated();

	// slots called from SCHNApps signals
	void map_added(Object*);
	void map_removed(Object*);

private:
	void selected_map_changed(const plugin_cmap_provider::CMap0Handler* h);
	void selected_map_changed(const plugin_cmap_provider::CMap2Handler* h);
	void selected_map_changed(const plugin_cmap_provider::CMap3Handler* h);
	void export_map(plugin_cmap_provider::CMap0Handler* h, cgogn::io::ExportOptions& opt);
	void export_map(plugin_cmap_provider::CMap2Handler* h, cgogn::io::ExportOptions& opt);
	void export_map(plugin_cmap_provider::CMap3Handler* h, cgogn::io::ExportOptions& opt);

	SCHNApps* schnapps_;
	Plugin_Export* plugin_;

	Object* selected_map_;
};

} // namespace plugin_export

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXPORT_DIALOG_H_
