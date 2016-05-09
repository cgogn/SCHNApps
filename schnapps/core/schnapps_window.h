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

#ifndef SCHNAPPS_CORE_SCHNAPPS_WINDOW_H_
#define SCHNAPPS_CORE_SCHNAPPS_WINDOW_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/schnapps.h>

#include <ui_schnapps.h>

#include <QDockWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>

namespace schnapps
{

class SCHNAPPS_CORE_API SCHNAppsWindow : public QMainWindow, public Ui::SCHNAppsWindow
{
	Q_OBJECT

	friend class SCHNApps;

public:

	SCHNAppsWindow(const QString& app_path) :
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

		schnapps_ = new SCHNApps(app_path, this);
	}

	~SCHNAppsWindow()
	{}

private slots:

	void about_SCHNApps()
	{
		QString str("SCHNApps:\nS... CGoGN Holder for Nice Applications\n"
					"Web site: http://cgogn.unistra.fr \n"
					"Contact information: cgogn@unistra.fr");
		QMessageBox::about(this, "About SCHNApps", str);
	}

	void about_CGoGN()
	{
		QString str("CGoGN:\nCombinatorial and Geometric modeling\n"
					"with Generic N-dimensional Maps\n"
					"Web site: http://cgogn.unistra.fr \n"
					"Contact information: cgogn@unistra.fr");
		QMessageBox::about(this, "About CGoGN", str);
	}

	void toggle_control_dock()
	{
		control_dock_->setVisible(control_dock_->isHidden());
	}

	void toggle_plugin_dock()
	{
		plugin_dock_->setVisible(plugin_dock_->isHidden());
	}

protected:

	void closeEvent(QCloseEvent *event)
	{
		schnapps_->schnapps_window_closing();
		QMainWindow::closeEvent(event);
	}

	SCHNApps* schnapps_;

	QDockWidget* control_dock_;
	QTabWidget* control_dock_tab_widget_;

	QDockWidget* plugin_dock_;
	QTabWidget* plugin_dock_tab_widget_;

	QVBoxLayout* central_layout_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SCHNAPPS_H_
