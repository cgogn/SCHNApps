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

#ifndef SCHNAPPS_CORE_CONTROL_DOCK_PLUGIN_TAB_H_
#define SCHNAPPS_CORE_CONTROL_DOCK_PLUGIN_TAB_H_

#include <schnapps/core/dll.h>

#include <ui_control_dock_plugin_tab_widget.h>

#include <QWidget>
#include <QString>

namespace schnapps
{

class SCHNApps;
class Plugin;

class SCHNAPPS_CORE_API ControlDock_PluginTab : public QWidget, public Ui::ControlDock_PluginTabWidget
{
	Q_OBJECT

public:

	ControlDock_PluginTab(SCHNApps* s);
	QString title() { return QString("Plugins"); }

private slots:

	// slots called from UI actions
	void add_plugin_directory_clicked();
	void enable_selected_plugins_clicked();
	void disable_selected_plugins_clicked();

	// slots called from SCHNApps signals
	void plugin_available_added(QString name);
	void plugin_enabled(Plugin* plugin);
	void plugin_disabled(Plugin* plugin);

private:

	SCHNApps* schnapps_;
	bool updating_ui_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CONTROL_DOCK_PLUGIN_TAB_H_
