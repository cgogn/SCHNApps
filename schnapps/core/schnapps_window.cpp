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

#include <schnapps/core/schnapps_window.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/settings_widget.h>
#include<QKeyEvent>


namespace schnapps
{

/**
 * @brief generate a SCHNApps_window (no need of schnapps_window.h !)
 */
SCHNAPPS_CORE_API std::unique_ptr<QMainWindow> schnapps_window_factory(const QString& app_path, const QString& settings_path)
{
	return cgogn::make_unique<SCHNAppsWindow>(app_path, settings_path);
}

SCHNAppsWindow::SCHNAppsWindow(const QString& app_path, const QString& settings_path) :
	QMainWindow()
{
	this->setupUi(this);

	// setup control dock

	control_dock_ = new QDockWidget("Control Dock", this);
	control_dock_->setAllowedAreas(Qt::LeftDockWidgetArea);
	control_dock_->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
	control_dock_->setMaximumWidth(250);

	control_dock_tab_widget_ = new QTabWidget(control_dock_);
	control_dock_tab_widget_->setObjectName("ControlDockTabWidget");
	control_dock_tab_widget_->setLayoutDirection(Qt::LeftToRight);
	control_dock_tab_widget_->setTabPosition(QTabWidget::North);
	control_dock_tab_widget_->setMovable(true);

	addDockWidget(Qt::LeftDockWidgetArea, control_dock_);
	control_dock_->setVisible(true);
	control_dock_->setWidget(control_dock_tab_widget_);

	connect(action_ToggleControlDock, SIGNAL(triggered()), this, SLOT(toggle_control_dock()));

	// setup plugin dock

	plugin_dock_ = new QDockWidget("Plugins Dock", this);
	plugin_dock_->setAllowedAreas(Qt::RightDockWidgetArea);
	plugin_dock_->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

	plugin_dock_tab_widget_ = new QTabWidget(plugin_dock_);
	plugin_dock_tab_widget_->setObjectName("PluginDockTabWidget");
	plugin_dock_tab_widget_->setLayoutDirection(Qt::LeftToRight);
	plugin_dock_tab_widget_->setTabPosition(QTabWidget::East);
	plugin_dock_tab_widget_->setMovable(true);

	addDockWidget(Qt::RightDockWidgetArea, plugin_dock_);
	plugin_dock_->setVisible(false);
	plugin_dock_->setWidget(plugin_dock_tab_widget_);

	connect(action_TogglePluginDock, SIGNAL(triggered()), this, SLOT(toggle_plugin_dock()));

	// setup central widget

	central_layout_ = new QVBoxLayout(centralwidget);
	central_layout_->setMargin(2);

	connect(action_AboutSCHNApps, SIGNAL(triggered()), this, SLOT(about_SCHNApps()));
	connect(action_AboutCGoGN, SIGNAL(triggered()), this, SLOT(about_CGoGN()));

	settings_widget_ = cgogn::make_unique<SettingsWidget>();
	connect(action_Settings, SIGNAL(triggered()), settings_widget_.get(), SLOT(display_setting_widget()));

	schnapps_ = cgogn::make_unique<SCHNApps>(app_path, settings_path, this);

	connect(action_ExportSettings, SIGNAL(triggered()), schnapps_.get(), SLOT(export_settings()));
}

SCHNAppsWindow::~SCHNAppsWindow()
{}

QAction* SCHNAppsWindow::add_menu_action(const QString& menu_path, const QString& action_text)
{
	QAction* action = nullptr;

	if (!menu_path.isEmpty())
	{
		// extracting all the substring separated by ';'
		QStringList step_names = menu_path.split(";");
		step_names.removeAll("");
		unsigned int nb_steps = step_names.count();

		// if only one substring: error + failure
		// No action directly in the menu bar
		if (nb_steps >= 1)
		{
			unsigned int i = 0;
			QMenu* last_menu = nullptr;
			for (QString step : step_names)
			{
				++i;
				if (i < nb_steps) // if not last substring (= menu)
				{
					// try to find an existing sub_menu with step name
					bool found = false;
					QList<QAction*> actions;
					if (i == 1) actions = menubar->actions();
					else actions = last_menu->actions();
					for (QAction* action : actions)
					{
						QMenu* sub_menu = action->menu();
						if (sub_menu && sub_menu->title() == step)
						{
							last_menu = sub_menu;
							found = true;
							break;
						}
					}
					if (!found)
					{
						QMenu* new_menu;
						if (i == 1)
						{
							new_menu = menubar->addMenu(step);
							new_menu->setParent(menubar);
						}
						else
						{
							new_menu = last_menu->addMenu(step);
							new_menu->setParent(last_menu);
						}
						last_menu = new_menu;
					}
				}
				else // if last substring (= action name)
				{
					action = new QAction(action_text, last_menu);
					last_menu->addAction(action);
					action->setText(step);

					// just to update the menu in buggy Qt5 on macOS
//					#if (defined CGOGN_APPLE) && ((QT_VERSION>>16) == 5)
//					QMenu* fakemenu = window_->menubar->addMenu("X");
//					delete fakemenu;
//					#endif
				}
			}
		}
	}

	return action;
}

void SCHNAppsWindow::remove_menu_action(QAction* action)
{
	// parent of the action
	// which is an instance of QMenu if the action was created
	// using the add_menu_action() method
	QObject* parent = action->parent();
	delete action;

	while(parent != nullptr)
	{
		QMenu* parent_menu = dynamic_cast<QMenu*>(parent);
		if (parent_menu && parent_menu->actions().empty())
		{
			parent = parent->parent();
			delete parent_menu;
		}
		else
			parent = nullptr;
	}
}

void SCHNAppsWindow::closeEvent(QCloseEvent *event)
{
	schnapps_->schnapps_window_closing();
	QMainWindow::closeEvent(event);
}

void SCHNAppsWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_V:
		schnapps_->cycle_selected_view();
		break;
	case Qt::Key_Escape:
		{
			QMessageBox msg_box;
			msg_box.setText("Really quit SCHNApps ?");
			msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msg_box.setDefaultButton(QMessageBox::Ok);
			if (msg_box.exec() == QMessageBox::Ok)
				schnapps_->close_window();
		}
		break;
	}
}

} // namespace schnapps
