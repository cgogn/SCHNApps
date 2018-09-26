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

#include <schnapps/core/control_dock_plugin_tab.h>

#include <QFileDialog>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/plugin.h>

namespace schnapps
{

ControlDock_PluginTab::ControlDock_PluginTab(SCHNApps* s) :
	schnapps_(s),
	updating_ui_(false)
{
	setupUi(this);

	// connect UI signals
	connect(button_addPluginDirectory, SIGNAL(clicked()), this, SLOT(add_plugin_directory_clicked()));
	connect(button_enablePlugins, SIGNAL(clicked()), this, SLOT(enable_selected_plugins_clicked()));
	connect(button_disablePlugins, SIGNAL(clicked()), this, SLOT(disable_selected_plugins_clicked()));
	connect(list_pluginsAvailable, &QListWidget::doubleClicked, [=](const QModelIndex& )
	{
		enable_selected_plugins_clicked();
	});
	connect(list_pluginsEnabled, &QListWidget::doubleClicked, [=](const QModelIndex& )
	{
		disable_selected_plugins_clicked();
	});

	// connect SCHNApps signals
	connect(schnapps_, SIGNAL(plugin_available_added(QString)), this, SLOT(plugin_available_added(QString)));
	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(plugin_enabled(Plugin*)));
	connect(schnapps_, SIGNAL(plugin_disabled(Plugin*)), this, SLOT(plugin_disabled(Plugin*)));
}





void ControlDock_PluginTab::add_plugin_directory_clicked()
{
	if (!updating_ui_)
	{
		QString dir = QFileDialog::getExistingDirectory(
			this,
			tr("Select a directory"),
			schnapps_->app_path(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);

		if (!dir.isEmpty())
			schnapps_->register_plugins_directory(dir);
	}
}

void ControlDock_PluginTab::enable_selected_plugins_clicked()
{
	if (!updating_ui_)
	{
		QList<QListWidgetItem*> items = list_pluginsAvailable->selectedItems();
		for (QListWidgetItem* item : items)
			schnapps_->enable_plugin(item->text());
	}
}

void ControlDock_PluginTab::disable_selected_plugins_clicked()
{
	if (!updating_ui_)
	{
		QList<QListWidgetItem*> items = list_pluginsEnabled->selectedItems();
		for (QListWidgetItem* item : items)
		{
			Plugin* plug = schnapps_->plugin(item->text());
			if (plug && !plug->auto_activate())
				schnapps_->disable_plugin(item->text());
		}
	}
}





void ControlDock_PluginTab::plugin_available_added(QString name)
{
	updating_ui_ = true;
	list_pluginsAvailable->addItem(name);
	updating_ui_ = false;
}

void ControlDock_PluginTab::plugin_enabled(Plugin *plugin)
{
	updating_ui_ = true;
	const QString& plugin_name = plugin->name();
	QList<QListWidgetItem*> av = list_pluginsAvailable->findItems(plugin_name, Qt::MatchExactly);
	if (!av.empty())
		delete av[0];
	list_pluginsEnabled->addItem(plugin_name);
	updating_ui_ = false;
}

void ControlDock_PluginTab::plugin_disabled(Plugin *plugin)
{
	updating_ui_ = true;
	const QString& plugin_name = plugin->name();
	QList<QListWidgetItem*> av = list_pluginsEnabled->findItems(plugin_name, Qt::MatchExactly);
	if (!av.empty())
		delete av[0];
	list_pluginsAvailable->addItem(plugin_name);
	updating_ui_ = false;
}

} // namespace schnapps
