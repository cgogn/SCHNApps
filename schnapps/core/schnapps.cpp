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

#include <schnapps/core/schnapps.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/view.h>

namespace schnapps
{

SCHNApps::SCHNApps(const QString& app_path) :
	QMainWindow(),
	app_path_(app_path),
	first_view_(NULL),
	selected_view_(NULL)
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

	first_view_ = add_view();
	set_selected_view(first_view_);
	root_splitter_->addWidget(first_view_);

	// connect basic actions

	connect(action_AboutSCHNApps, SIGNAL(triggered()), this, SLOT(about_SCHNApps()));
	connect(action_AboutCGoGN, SIGNAL(triggered()), this, SLOT(about_CGoGN()));
}

SCHNApps::~SCHNApps()
{}

/*********************************************************
 * MANAGE CAMERAS
 *********************************************************/

Camera* SCHNApps::add_camera(const QString& name)
{
	if (cameras_.contains(name))
		return NULL;

	Camera* camera = new Camera(name, this);
	cameras_.insert(name, camera);
	emit(camera_added(camera));
	return camera;
}

Camera* SCHNApps::add_camera()
{
	return add_camera(QString("camera_") + QString::number(Camera::camera_count_));
}

void SCHNApps::remove_camera(const QString& name)
{
	Camera* camera = get_camera(name);
	if (camera && !camera->is_used())
	{
		cameras_.remove(name);
		emit(camera_removed(camera));
		delete camera;
	}
}

Camera* SCHNApps::get_camera(const QString& name) const
{
	if (cameras_.contains(name))
		return cameras_[name];
	else
		return NULL;
}

/*********************************************************
 * MANAGE VIEWS
 *********************************************************/


View* SCHNApps::add_view(const QString& name)
{
	if (views_.contains(name))
		return NULL;

	View* view = new View(name, this);
	views_.insert(name, view);
	emit(view_added(view));
	return view;
}

View* SCHNApps::add_view()
{
	return add_view(QString("view_") + QString::number(View::view_count_));
}

void SCHNApps::remove_view(const QString& name)
{
	if (views_.contains(name))
	{
		if(views_.count() > 1)
		{
			View* view = views_[name];
			if(view == first_view_)
			{
				QMap<QString, View*>::const_iterator it = views_.constBegin();
				while (it != views_.constEnd())
				{
					if(it.value() != view)
					{
						first_view_ = it.value();
						it = views_.constEnd();
					}
					else
						++it;
				}
			}
			if(view == selected_view_)
				set_selected_view(first_view_);

			views_.remove(name);
			emit(view_removed(view));
			delete view;
		}
	}
}

View* SCHNApps::get_view(const QString& name) const
{
	if (views_.contains(name))
		return views_[name];
	else
		return NULL;
}

void SCHNApps::set_selected_view(View* view)
{
//	int current_tab = plugin_dock_tab_widget_->currentIndex();

//	if(selected_view_)
//	{
//		foreach(PluginInteraction* p, selected_view_->get_linked_plugins())
//			disable_plugin_tab_widgets(p);
//		disconnect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
//		disconnect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));
//	}

	View* old_selected = selected_view_;
	selected_view_ = view;
	if (old_selected)
		old_selected->hide_dialogs();

//	foreach(PluginInteraction* p, selected_view_->get_linked_plugins())
//		enable_plugin_tab_widgets(p);
//	connect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
//	connect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));

//	plugin_dock_tab_widget_->setCurrentIndex(current_tab);

	emit(selected_view_changed(old_selected, selected_view_));

	if(old_selected)
		old_selected->update();
	selected_view_->update();
}

void SCHNApps::set_selected_view(const QString& name)
{
	View* v = this->get_view(name);
	set_selected_view(v);
}

View* SCHNApps::split_view(const QString& name, Qt::Orientation orientation)
{
	View* new_view = add_view();

	View* view = views_[name];
	QSplitter* parent = static_cast<QSplitter*>(view->parentWidget());

	if(parent == root_splitter_ && !root_splitter_initialized_)
	{
		root_splitter_->setOrientation(orientation);
		root_splitter_initialized_ = true;
	}

	if (parent->orientation() == orientation)
	{
		parent->insertWidget(parent->indexOf(view) + 1, new_view);
		QList<int> sz = parent->sizes();
		int tot = 0;
		for (int i = 0; i < parent->count(); ++i)
			tot += sz[i];
		sz[0] = tot / parent->count() + tot % parent->count();
		for (int i = 1; i < parent->count(); ++i)
			sz[i] = tot / parent->count();
		parent->setSizes(sz);
	}
	else
	{
		int idx = parent->indexOf(view);
		view->setParent(NULL);
		QSplitter* spl = new QSplitter(orientation);
		spl->addWidget(view);
		spl->addWidget(new_view);
		parent->insertWidget(idx, spl);

		QList<int> sz = spl->sizes();
		int tot = sz[0] + sz[1];
		sz[0] = tot / 2;
		sz[1] = tot - sz[0];
		spl->setSizes(sz);
	}

	return new_view;
}

QString SCHNApps::get_split_view_positions()
{
	QList<QSplitter*> liste;
	liste.push_back(root_splitter_);

	QString result;
	QTextStream qts(&result);
	while (!liste.empty())
	{
		QSplitter* spl = liste.first();
		for (int i = 0; i < spl->count(); ++i)
		{
			QWidget* w = spl->widget(i);
			QSplitter* qw = dynamic_cast<QSplitter*>(w);
			if (qw != NULL)
				liste.push_back(qw);
		}
		QByteArray ba = spl->saveState();
		qts << ba.count() << " ";
		for (int j = 0; j < ba.count(); ++j)
			qts << int(ba[j]) << " ";
		liste.pop_front();
	}
	return result;
}

void SCHNApps::set_split_view_positions(QString positions)
{
	QList<QSplitter*> liste;
	liste.push_back(root_splitter_);

	QTextStream qts(&positions);
	while (!liste.empty())
	{
		QSplitter* spl = liste.first();
		for (int i = 0; i < spl->count(); ++i)
		{
			QWidget *w = spl->widget(i);
			QSplitter* qw = dynamic_cast<QSplitter*>(w);
			if (qw != NULL)
				liste.push_back(qw);
		}
		if (qts.atEnd())
		{
			std::cerr << "Problem restoring view split configuration" << std::endl;
			return;
		}

		int nb;
		qts >> nb;
		QByteArray ba(nb + 1, 0);
		for (int j = 0; j < nb; ++j)
		{
			int v;
			qts >> v;
			ba[j] = char(v);
		}
		spl->restoreState(ba);
		liste.pop_front();
	}
}



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
