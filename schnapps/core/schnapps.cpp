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
	first_view_(nullptr),
	selected_view_(nullptr),
	window_(window)
{
	status_bar_output_ = cgogn::make_unique<StatusBarOutput>(window->statusBar());
	cgogn::logger::Logger::get_logger().add_output(status_bar_output_.get());

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

#ifdef WIN32
	register_plugins_directory(app_path_);
#else
	register_plugins_directory(app_path_ + QString("/../lib"));
#endif

	settings_ = Settings::from_file(app_path_ + QString("/../lib/settings.json"));
	settings_->set_widget(window->settings_widget_.get());
	for (const QVariant& plugin_dir_v : get_core_setting("Plugins paths").toList())
		this->register_plugins_directory(plugin_dir_v.toString());
	for (const QVariant& plugin_v : get_core_setting("Load modules").toList())
		this->enable_plugin(plugin_v.toString());
}

SCHNApps::~SCHNApps()
{
	settings_->to_file(app_path_ + QString("/../lib/settings.json"));
	// first safely unload every plugins (this has to be done before the views get deleted)
	while (!plugins_.empty())
		this->disable_plugin(plugins_.begin()->first);
}

/*********************************************************
 * MANAGE CAMERAS
 *********************************************************/

Camera* SCHNApps::add_camera(const QString& name)
{
	if (cameras_.count(name) > 0ul)
		return nullptr;

	cameras_.insert(std::make_pair(name, cgogn::make_unique<Camera>(name, this)));
	Camera* camera = cameras_.at(name).get();
	emit(camera_added(camera));
	return camera;
}

Camera* SCHNApps::add_camera()
{
	return add_camera(QString("camera_") + QString::number(Camera::camera_count_));
}

void SCHNApps::remove_camera(const QString& name)
{
	if (cameras_.count(name) > 0ul)
	{
		auto camera = std::move(cameras_.at(name));
		if (!camera->is_used())
		{
			cameras_.erase(name);
			emit(camera_removed(camera.get()));
		}
	}
}

Camera* SCHNApps::get_camera(const QString& name) const
{
	if (cameras_.count(name) > 0ul)
		return cameras_.at(name).get();
	else
		return nullptr;
}

/*********************************************************
 * MANAGE PLUGINS
 *********************************************************/

void SCHNApps::register_plugins_directory(const QString& path)
{
	QDir directory(QDir::cleanPath(path));
	if (directory.exists())
	{
		QStringList filters;
		filters << "lib*plugin*.so";
		filters << "lib*plugin*.dylib";
		filters << "*plugin*.dll";

		QStringList plugin_files = directory.entryList(filters, QDir::Files);

		for (const QString& plugin_file : plugin_files)
		{
			QFileInfo pfi(plugin_file);
#ifdef WIN32
			QString plugin_name = pfi.baseName();
#else
			QString plugin_name = pfi.baseName().remove(0, 3);
#endif
			if (plugin_name.endsWith("_d"))
				plugin_name = plugin_name.left(plugin_name.size() -2);
			if(plugin_name.startsWith("plugin_",  Qt::CaseInsensitive))
				plugin_name = plugin_name.right(plugin_name.size() - 7);

			QString plugin_file_path = directory.absoluteFilePath(plugin_file);

			if (available_plugins_.count(plugin_name) == 0ul)
			{
				available_plugins_.insert(std::make_pair(plugin_name, plugin_file_path));
				emit(plugin_available_added(plugin_name));
			}
			else
			{
				cgogn_log_info("SCHNApps::register_plugins_directory") << "Plugin \"" <<  plugin_name.toStdString() << "\" already loaded.";
			}
		}
	}
}

Plugin* SCHNApps::enable_plugin(const QString& plugin_name)
{
	if (plugins_.count(plugin_name) > 0ul)
		return plugins_.at(plugin_name).get();

	if (available_plugins_.count(plugin_name) > 0ul)
	{
		QString plugin_file_path = available_plugins_[plugin_name];
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
				plugins_.insert(std::make_pair(plugin_name, std::unique_ptr<Plugin>(plugin)));

				cgogn_log_info("SCHNApps::enable_plugin") << (plugin_name + QString(" successfully loaded.")).toStdString();
				emit(plugin_enabled(plugin));
//				menubar->repaint();
				return plugin;
			}
			else
			{
				delete plugin;
				return nullptr;
			}
		} else { // if loading fails
			cgogn_log_warning("SCHNApps::enable_plugin") << "Loader.instance() failed with error \"" << loader.errorString().toStdString() << "\".";
			return nullptr;
		}
	}
	else
		return nullptr;
}

void SCHNApps::disable_plugin(const QString& plugin_name)
{
	if (plugins_.count(plugin_name) > 0ul)
	{
		auto plugin = std::move(plugins_.at(plugin_name));

		// unlink linked views (for interaction plugins)
		PluginInteraction* pi = dynamic_cast<PluginInteraction*>(plugin.get());
		if (pi)
		{
			while(!pi->get_linked_views().empty()) // Safe way to iterate over a container that is currently being modified
				(*pi->get_linked_views().begin())->unlink_plugin(pi);
		}

		// call disable() method and dereference plugin
		plugin->disable();
		plugins_.erase(plugin_name);

		QPluginLoader loader(plugin->get_file_path());
		loader.unload();

		cgogn_log_info("SCHNApps::disable_plugin") << plugin_name.toStdString()  << " successfully unloaded.";
		emit(plugin_disabled(plugin.get()));
	}
}

Plugin* SCHNApps::get_plugin(const QString& name) const
{
	if (plugins_.count(name) > 0ul)
		return plugins_.at(name).get();
	else
		return nullptr;
}

void SCHNApps::add_plugin_dock_tab(Plugin* plugin, QWidget* tab_widget, const QString& tab_text)
{
	std::list<QWidget*>& widget_list = plugin_dock_tabs_[plugin];
	if (plugin && tab_widget && std::find(widget_list.begin(), widget_list.end(), tab_widget) == widget_list.end())
	{
		int current_tab = window_->plugin_dock_tab_widget_->currentIndex();

		int idx = window_->plugin_dock_tab_widget_->addTab(tab_widget, tab_text);
		window_->plugin_dock_->setVisible(true);

		PluginInteraction* pi = dynamic_cast<PluginInteraction*>(plugin);
		if (pi)
		{
			if (pi->is_linked_to_view(selected_view_))
				window_->plugin_dock_tab_widget_->setTabEnabled(idx, true);
			else
				window_->plugin_dock_tab_widget_->setTabEnabled(idx, false);
		}

		if (current_tab != -1)
			window_->plugin_dock_tab_widget_->setCurrentIndex(current_tab);

		widget_list.push_back(tab_widget);
	}
}

void SCHNApps::remove_plugin_dock_tab(Plugin* plugin, QWidget *tab_widget)
{
	std::list<QWidget*>& widget_list = plugin_dock_tabs_[plugin];
	auto widget_it = std::find(widget_list.begin(), widget_list.end(), tab_widget);
	if (plugin && tab_widget && widget_it != widget_list.end())
	{
		window_->plugin_dock_tab_widget_->removeTab(window_->plugin_dock_tab_widget_->indexOf(tab_widget));
		widget_list.erase(widget_it);
	}
}

void SCHNApps::enable_plugin_tab_widgets(Plugin* plugin)
{
	if (plugin_dock_tabs_.count(plugin) > 0ul)
	{
		for (const auto& widget : plugin_dock_tabs_.at(plugin))
			window_->plugin_dock_tab_widget_->setTabEnabled(window_->plugin_dock_tab_widget_->indexOf(widget), true);
	}
}

void SCHNApps::disable_plugin_tab_widgets(Plugin* plugin)
{
	if (plugin_dock_tabs_.count(plugin) > 0ul)
	{
		for (const auto& widget : plugin_dock_tabs_.at(plugin))
			window_->plugin_dock_tab_widget_->setTabEnabled(window_->plugin_dock_tab_widget_->indexOf(widget), false);
	}
}

/*********************************************************
 * MANAGE MAPS
 *********************************************************/

MapHandlerGen* SCHNApps::add_map(const QString &name, unsigned int dimension)
{
	QString final_name = name;
	if (maps_.count(name) > 0ul)
	{
		int i = 1;
		do
		{
			final_name = name + QString("_") + QString::number(i);
			++i;
		} while (maps_.count(final_name) > 0ul);
	}

	switch(dimension)
	{
		case 2 : {
			maps_.insert(std::make_pair(final_name, cgogn::make_unique<CMap2Handler>(final_name, this)));
			break;
		}
		case 3 : {
			maps_.insert(std::make_pair(final_name, cgogn::make_unique<CMap3Handler>(final_name, this)));
			break;
		}
	}

	MapHandlerGen* mhg = maps_.at(final_name).get();
	emit(map_added(mhg));

	return mhg;
}

void SCHNApps::remove_map(const QString &name)
{
	if (maps_.count(name) > 0ul)
	{
		auto& map = maps_.at(name);
		auto views = map->get_linked_views(); // do NOT use a ref-to-const here because the referenced list is modified in the following loop
		for (View* view : views)
		{
			view->unlink_map(map.get());
		}
		emit(map_removed(map.get()));
		maps_.erase(name);
	}
}

MapHandlerGen* SCHNApps::duplicate_map(const QString& name)
{
	if (maps_.count(name) > 0ul)
	{
		auto& map = maps_.at(name);
		MapHandlerGen* duplicate = this->add_map(QString("copy_") + map->get_name(), map->dimension());
		duplicate->merge(map.get());
		return duplicate;
	} else {
		return nullptr;
	}
}



MapHandlerGen* SCHNApps::get_map(const QString& name) const
{
	if (maps_.count(name) > 0ul)
		return maps_.at(name).get();
	else
		return nullptr;
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
	if (views_.count(name) > 0ul)
		return nullptr;

	views_.insert(std::make_pair(name, cgogn::make_unique<View>(name, this)));
	View* view = views_.at(name).get();
	emit(view_added(view));
	return view;
}

View* SCHNApps::add_view()
{
	return add_view(QString("view_") + QString::number(View::view_count_));
}

void SCHNApps::remove_view(const QString& name)
{
	if (views_.count(name) > 0ul)
	{
		if (views_.size() > 1)
		{
			auto view = std::move(views_.at(name));
			views_.erase(name);
			if (view.get() == first_view_)
				first_view_ = views_.begin()->second.get();

			if (view.get() == selected_view_)
				set_selected_view(first_view_);

			emit(view_removed(view.get()));
		}
	}
}

View* SCHNApps::get_view(const QString& name) const
{
	if (views_.count(name) > 0ul)
		return views_.at(name).get();
	else
		return nullptr;
}

void SCHNApps::set_selected_view(View* view)
{
	int current_tab = window_->plugin_dock_tab_widget_->currentIndex();

//	if (selected_view_)
//	{
//		for (PluginInteraction* p : selected_view_->get_linked_plugins())
//			disable_plugin_tab_widgets(p);
//		disconnect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
//		disconnect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));
//	}

	View* old_selected = selected_view_;
	selected_view_ = view;
	if (old_selected)
		old_selected->hide_dialogs();

//	for (PluginInteraction* p : selected_view_->get_linked_plugins())
//		enable_plugin_tab_widgets(p);
//	connect(selected_view_, SIGNAL(plugin_linked(PluginInteraction*)), this, SLOT(enable_plugin_tab_widgets(PluginInteraction*)));
//	connect(selected_view_, SIGNAL(plugin_unlinked(PluginInteraction*)), this, SLOT(disable_plugin_tab_widgets(PluginInteraction*)));

	window_->plugin_dock_tab_widget_->setCurrentIndex(current_tab);

	emit(selected_view_changed(old_selected, selected_view_));

	if (old_selected)
		old_selected->update();
	selected_view_->update();
}

void SCHNApps::set_selected_view(const QString& name)
{
	View* v = this->get_view(name);
	if (v)
		set_selected_view(v);
}

View* SCHNApps::split_view(const QString& name, Qt::Orientation orientation)
{
	View* new_view = add_view();

	View* view = views_.at(name).get();
	QSplitter* parent = static_cast<QSplitter*>(view->parentWidget());

	if (parent == root_splitter_ && !root_splitter_initialized_)
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
		view->setParent(nullptr);
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
			if (qw != nullptr)
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
			if (qw != nullptr)
				liste.push_back(qw);
		}
		if (qts.atEnd())
		{
			cgogn_log_error("SCHNApps::set_split_view_positions") << "Problem restoring view split configuration.";
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
