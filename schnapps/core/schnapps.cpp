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

#include <QVBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QDockWidget>
#include <QPluginLoader>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFile>
#include <QByteArray>

#include <core/schnapps.h>

namespace schnapps
{

SCHNApps::SCHNApps(const QString& app_path) :
	QMainWindow(),
	app_path_(app_path)
{
	this->setupUi(this);

	// create & setup control dock

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

	// control_camera_tab_ = new ControlDock_CameraTab(this);
	// control_dock_tab_widget_->addTab(control_camera_tab_, control_camera_tab_->title());
	// control_map_tab_ = new ControlDock_MapTab(this);
	// control_dock_tab_widget_->addTab(control_map_tab_, control_map_tab_->title());
	// control_plugin_tab_ = new ControlDock_PluginTab(this);
	// control_dock_tab_widget_->addTab(control_plugin_tab_, control_plugin_tab_->title());

	connect(action_ToggleControlDock, SIGNAL(triggered()), this, SLOT(toggle_control_dock()));

	// create & setup central widget (views)
	
	central_layout_ = new QVBoxLayout(centralwidget);
	central_layout_->setMargin(2);

	root_splitter_ = new QSplitter(centralwidget);
	root_splitter_initialized_ = false;
	central_layout_->addWidget(root_splitter_);

	// connect basic actions

	connect(action_AboutSCHNApps, SIGNAL(triggered()), this, SLOT(about_SCHNApps()));
	connect(action_AboutCGoGN, SIGNAL(triggered()), this, SLOT(about_CGoGN()));
}

SCHNApps::~SCHNApps()
{}

void SCHNApps::about_SCHNApps()
{
	QString str("SCHNApps:\nS... CGoGN Holder for Nice Applications\n"
	            "Web site: http://cgogn.unistra.fr \n"
	            "Contact information: cgogn@unistra.fr");
	QMessageBox::about(this, "About SCHNApps", str);
}

void SCHNApps::about_CGoGN()
{
	QString str("CGoGN:\nCombinatorial and Geometric modeling\n"
	            "with Generic N-dimensional Maps\n"
	            "Web site: http://cgogn.unistra.fr \n"
	            "Contact information: cgogn@unistra.fr");
	QMessageBox::about(this, "About CGoGN", str);
}

void SCHNApps::toggle_control_dock()
{
	control_dock_->setVisible(control_dock_->isHidden());
}

void SCHNApps::closeEvent(QCloseEvent *event)
{
	emit(schnapps_closing());
	QMainWindow::closeEvent(event);
}

void SCHNApps::status_bar_message(const QString& msg, int msec)
{
	statusbar->showMessage(msg, msec);
}

} // namespace schnapps
