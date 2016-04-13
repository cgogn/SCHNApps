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

#include <schnapps/core/schnapps.h>

#include <schnapps/core/camera.h>
#include <schnapps/core/view.h>
#include <schnapps/core/plugin.h>
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/map_handler.h>

#include <schnapps/core/control_dock_camera_tab.h>
#include <schnapps/core/control_dock_plugin_tab.h>
#include <schnapps/core/control_dock_map_tab.h>

#include <schnapps/core/schnapps_window.h>

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
#include <QAction>

namespace schnapps
{

SCHNApps::SCHNApps(const QString& app_path, SCHNAppsWindow* window) :
	app_path_(app_path),
	first_view_(NULL),
	selected_view_(NULL),
	window_(window)
{
	// create & setup control dock

	control_camera_tab_ = new ControlDock_CameraTab(this);
	window_->control_dock_tab_widget_->addTab(control_camera_tab_, control_camera_tab_->title());
	control_plugin_tab_ = new ControlDock_PluginTab(this);
	window_->control_dock_tab_widget_->addTab(control_plugin_tab_, control_plugin_tab_->title());
	control_map_tab_ = new ControlDock_MapTab(this);
	window_->control_dock_tab_widget_->addTab(control_map_tab_, control_map_tab_->title());

	// create & setup central widget (views)

	root_splitter_ = new QSplitter(window_->centralwidget);
	root_splitter_initialized_ = false;
	window_->central_layout_->addWidget(root_splitter_);

	first_view_ = add_view();
	set_selected_view(first_view_);
	root_splitter_->addWidget(first_view_);

	register_plugins_directory(app_path + QString("/../lib"));
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
 * MANAGE PLUGINS
 *********************************************************/

void SCHNApps::register_plugins_directory(const QString& path)
{
#ifdef WIN32
#ifdef _DEBUG
	QDir directory(path + QString("Debug/"));
#else
	QDir directory(path + QString("Release/"));
#endif
#else
	QDir directory(path);
#endif
	if (directory.exists())
	{
		QStringList filters;
		filters << "lib*plugin*.so";
		filters << "lib*plugin*.dylib";
		filters << "*plugin*.dll";

		QStringList plugin_files = directory.entryList(filters, QDir::Files);

		foreach (QString plugin_file, plugin_files)
		{
			QFileInfo pfi(plugin_file);
#ifdef WIN32
			QString plugin_name = pfi.baseName();
#else
			QString plugin_name = pfi.baseName().remove(0, 3);
#endif
			QString plugin_file_path = directory.absoluteFilePath(plugin_file);

			if(!available_plugins_.contains(plugin_name))
			{
				available_plugins_.insert(plugin_name, plugin_file_path);
				emit(plugin_available_added(plugin_name));
			}
		}
	}
}

Plugin* SCHNApps::enable_plugin(const QString& plugin_name)
{
	if (plugins_.contains(plugin_name))
		return plugins_[plugin_name];

	if (available_plugins_.contains(plugin_name))
	{
		QString plugin_file_path = available_plugins_[plugin_name];

		std::cout << plugin_file_path.toStdString() << std::endl;
		QPluginLoader loader(plugin_file_path);

		// if the loader loads a plugin instance
		if (QObject* plugin_object = loader.instance())
		{
			Plugin* plugin = qobject_cast<Plugin*>(plugin_object);

			// set the plugin with correct parameters (name, filepath, SCHNApps)
			plugin->set_name(plugin_name);
			plugin->set_file_path(plugin_file_path);
			plugin->set_schnapps(this);

			// then we call its enable() methods
			if (plugin->enable())
			{
				// if it succeeded we reference this plugin
				plugins_.insert(plugin_name, plugin);

				status_bar_message(plugin_name + QString(" successfully loaded."), 2000);
				emit(plugin_enabled(plugin));
//				menubar->repaint();
				return plugin;
			}
			else
			{
				delete plugin;
				return NULL;
			}
		}
		// if loading fails
		else
		{
			std::cout << "loader.instance() failed.." << std::endl;
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

void SCHNApps::disable_plugin(const QString& plugin_name)
{
	if (plugins_.contains(plugin_name))
	{
		Plugin* plugin = plugins_[plugin_name];

		// unlink linked views (for interaction plugins)
		PluginInteraction* pi = dynamic_cast<PluginInteraction*>(plugin);
		if(pi)
		{
			foreach (View* view, pi->get_linked_views())
				view->unlink_plugin(pi);
		}

		// call disable() method and dereference plugin
		plugin->disable();
		plugins_.remove(plugin_name);

//		// remove plugin dock tabs
//		foreach (QWidget* tab, plugin_tabs_[plugin])
//			remove_plugin_dock_tab(plugin, tab);
		// remove plugin menu actions
		foreach (QAction* action, plugin_menu_actions_[plugin])
			remove_menu_action(plugin, action);

		QPluginLoader loader(plugin->get_file_path());
		loader.unload();

		status_bar_message(plugin_name + QString(" successfully unloaded."), 2000);
		emit(plugin_disabled(plugin));

		delete plugin;
	}
}

Plugin* SCHNApps::get_plugin(const QString& name) const
{
	if (plugins_.contains(name))
		return plugins_[name];
	else
		return NULL;
}

//void SCHNApps::add_plugin_dock_tab(Plugin* plugin, QWidget* tabWidget, const QString& tabText)
//{
//	if(tabWidget && !m_pluginTabs[plugin].contains(tabWidget))
//	{
//		int currentTab = m_pluginDockTabWidget->currentIndex();

//		int idx = m_pluginDockTabWidget->addTab(tabWidget, tabText);
//		m_pluginDock->setVisible(true);

//		PluginInteraction* pi = dynamic_cast<PluginInteraction*>(plugin);
//		if(pi)
//		{
//			if(pi->isLinkedToView(m_selectedView))
//				m_pluginDockTabWidget->setTabEnabled(idx, true);
//			else
//				m_pluginDockTabWidget->setTabEnabled(idx, false);
//		}

//		if(currentTab != -1)
//			m_pluginDockTabWidget->setCurrentIndex(currentTab);

//		m_pluginTabs[plugin].append(tabWidget);
//	}
//}

//void SCHNApps::remove_plugin_dock_tab(Plugin* plugin, QWidget *tabWidget)
//{
//	if(tabWidget && m_pluginTabs[plugin].contains(tabWidget))
//	{
//		m_pluginDockTabWidget->removeTab(m_pluginDockTabWidget->indexOf(tabWidget));

//		m_pluginTabs[plugin].removeOne(tabWidget);
//	}
//}

//void SCHNApps::enable_plugin_tab_widgets(PluginInteraction* plugin)
//{
//	if(m_pluginTabs.contains(plugin))
//	{
//		foreach (QWidget* w, m_pluginTabs[plugin])
//			m_pluginDockTabWidget->setTabEnabled(m_pluginDockTabWidget->indexOf(w), true);
//	}
//}

//void SCHNApps::disable_plugin_tab_widgets(PluginInteraction* plugin)
//{
//	if(m_pluginTabs.contains(plugin))
//	{
//		foreach (QWidget* w, m_pluginTabs[plugin])
//			m_pluginDockTabWidget->setTabEnabled(m_pluginDockTabWidget->indexOf(w), false);
//	}
//}

/*********************************************************
 * MANAGE MAPS
 *********************************************************/

MapHandlerGen* SCHNApps::add_map(const QString &name, unsigned int dimension)
{
	QString final_name = name;
	if (maps_.contains(name))
	{
		int i = 1;
		do
		{
			final_name = name + QString("_") + QString::number(i);
			++i;
		} while (maps_.contains(final_name));
	}

	MapHandlerGen* mh = NULL;
	switch(dimension)
	{
		case 2 : {
			CMap2* map = new CMap2();
			mh = new MapHandler<CMap2>(final_name, this, map);
			break;
		}
		case 3 : {
			CMap3* map = new CMap3();
			mh = new MapHandler<CMap3>(final_name, this, map);
			break;
		}
	}

	maps_.insert(final_name, mh);
	emit(map_added(mh));

	return mh;
}

void SCHNApps::remove_map(const QString &name)
{
	if (maps_.contains(name))
	{
		MapHandlerGen* map = maps_[name];
		foreach (View* view, map->get_linked_views())
			view->unlink_map(map);

		maps_.remove(name);
		emit(map_removed(map));

		// unselect map if it is removed
		if (this->get_selected_map() == map)
			set_selected_map(QString("NONE"));

		delete map;
	}
}

MapHandlerGen* SCHNApps::get_map(const QString& name) const
{
	if (maps_.contains(name))
		return maps_[name];
	else
		return NULL;
}

MapHandlerGen* SCHNApps::get_selected_map() const
{
	return control_map_tab_->get_selected_map();
}

void SCHNApps::set_selected_map(const QString& name)
{
	control_map_tab_->set_selected_map(name);
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
	int current_tab = window_->plugin_dock_tab_widget_->currentIndex();

	if(selected_view_)
	{
//		foreach (PluginInteraction* p, selected_view_->get_linked_plugins())
//			disable_plugin_tab_widgets(p);
		disconnect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
		disconnect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));
	}

	View* old_selected = selected_view_;
	selected_view_ = view;
	if (old_selected)
		old_selected->hide_dialogs();

//	foreach (PluginInteraction* p, selected_view_->get_linked_plugins())
//		enable_plugin_tab_widgets(p);
	connect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
	connect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));

	window_->plugin_dock_tab_widget_->setCurrentIndex(current_tab);

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

/*********************************************************
 * MANAGE MENU ACTIONS
 *********************************************************/

void SCHNApps::add_menu_action(Plugin* plugin, const QString& menu_path, QAction* action)
{
	if(action && !menu_path.isEmpty() && !plugin_menu_actions_[plugin].contains(action))
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
			QMenu* last_menu = NULL;
			foreach(QString step, step_names)
			{
				++i;
				if (i < nb_steps) // if not last substring (= menu)
				{
					// try to find an existing sub_menu with step name
					bool found = false;
					QList<QAction*> actions;
					if(i == 1) actions = window_->menubar->actions();
					else actions = last_menu->actions();
					foreach(QAction* action, actions)
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
						if(i == 1)
						{
							new_menu = window_->menubar->addMenu(step);
							new_menu->setParent(window_->menubar);
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
					last_menu->addAction(action);
					action->setText(step);
					action->setParent(last_menu);
				}
			}
		}

		// just to update the menu in buggy Qt5 on macOS
//		#if (defined CGOGN_APPLE) && ((QT_VERSION>>16) == 5)
//		QMenu* fakemenu = menuBar()->addMenu("X");
//		delete fakemenu;
//		#endif

		plugin_menu_actions_[plugin].append(action);
	}
}

void SCHNApps::remove_menu_action(Plugin* plugin, QAction* action)
{
	if(action)
	{
		action->setEnabled(false);

		plugin_menu_actions_[plugin].removeOne(action);

		// parent of the action
		// which is an instance of QMenu if the action was created
		// using the add_menu_action() method
		QObject* parent = action->parent();
		delete action;

		while(parent != NULL)
		{
			QMenu* parent_menu = dynamic_cast<QMenu*>(parent);
			if(parent_menu && parent_menu->actions().empty())
			{
				parent = parent->parent();
				delete parent_menu;
			}
			else
				parent = NULL;
		}
	}
}

/*********************************************************
 * MANAGE WINDOW
 *********************************************************/

void SCHNApps::close_window()
{
	window_->close();
}

void SCHNApps::schnapps_window_closing()
{
	emit(schnapps_closing());
}

void SCHNApps::status_bar_message(const QString& msg, int msec)
{
	window_->statusbar->showMessage(msg, msec);
}

void SCHNApps::set_window_size(int w, int h)
{
	window_->resize(w, h);
}

} // namespace schnapps
