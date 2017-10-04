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

#ifndef SCHNAPPS_CORE_SCHNAPPS_H_
#define SCHNAPPS_CORE_SCHNAPPS_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/settings.h>
#include <schnapps/core/status_bar_output.h>

#include <cgogn/core/utils/type_traits.h>

#include <QObject>
#include <QString>

class QSplitter;
class QAction;

namespace schnapps
{

class Camera;
class View;
class Plugin;
class PluginInteraction;
class MapHandlerGen;

class SCHNAppsWindow;

class ControlDock_CameraTab;
class ControlDock_MapTab;
class ControlDock_PluginTab;

/**
 * @brief The SCHNApps central object application
 */
class SCHNAPPS_CORE_API SCHNApps : public QObject
{
	Q_OBJECT

public:

	SCHNApps(const QString& app_path, const QString& settings_path, SCHNAppsWindow* window);
	~SCHNApps();

public slots:

	/**
	 * @brief get the file path where application has been launched
	 * @return the path
	 */
	inline const QString& get_app_path() { return app_path_; }

	/*********************************************************
	 * MANAGE CAMERAS
	 *********************************************************/

	/**
	* @brief add a camera with a given name
	* @param name name of camera
	*/
	Camera* add_camera(const QString& name);

	/**
	 * @brief add a camera (name automatically)
	 * @return
	 */
	Camera* add_camera();

	/**
	* @brief remove a camera
	* @param name name of camera to remove
	*/
	void remove_camera(const QString& name);

	/**
	* @brief get camera object
	* @param name of camera
	*/
	Camera* get_camera(const QString& name) const;

public:

	template <typename FUNC>
	void foreach_camera(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, Camera*>::value, "Wrong function parameter type");
		for (const auto& camera_it : cameras_)
			f(camera_it.second.get());
	}

	/*********************************************************
	 * MANAGE PLUGINS
	 *********************************************************/

public slots:

	/**
	* @brief Add a directory for searching available plugins
	* @param path path to directory
	*/
	void register_plugins_directory(const QString& path);

	/**
	* @brief Load and enable a plugin
	* @param plugin_name plugin name
	*/
	Plugin* enable_plugin(const QString& plugin_name);

	/**
	* @brief Disable and unload a plugin
	* @param plugin_name plugin name
	*/
	void disable_plugin(const QString& plugin_name);

	/**
	* @brief Get plugin object from name
	* @param name name of plugin
	*/
	Plugin* get_plugin(const QString& name) const;

	// get a set of available plugins
	inline const std::map<QString, QString>& get_available_plugins() const { return available_plugins_; }

public:

	template <typename FUNC>
	void foreach_plugin(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, Plugin*>::value, "Wrong function parameter type");
		for (const auto& plugin_it : plugins_)
			f(plugin_it.second.get());
	}

	void add_plugin_dock_tab(Plugin* plugin, QWidget* tab_widget, const QString& tab_text);

	void remove_plugin_dock_tab(Plugin* plugin, QWidget* tab_widget);

public slots:

	void enable_plugin_tab_widgets(Plugin* plugin);

	void disable_plugin_tab_widgets(Plugin* plugin);

	/*********************************************************
	 * MANAGE MAPS
	 *********************************************************/

	/**
	* @brief add a new empty map
	* @param name name given to the map
	* @param dimension dimension of the map
	*/
	MapHandlerGen* add_map(const QString& name, unsigned int dimension);

	/**
	* @brief Remove a map
	* @param name name of map
	*/
	void remove_map(const QString& name);

	/**
	* @brief Duplicate (copy) a map
	* @param name of map to copy
	*/
	MapHandlerGen* duplicate_map(const QString& name);

	/**
	* @brief Get a map object from its name
	* @param name name of map
	*/
	MapHandlerGen* get_map(const QString& name) const;

public:

	template <typename FUNC>
	void foreach_map(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, MapHandlerGen*>::value, "Wrong function parameter type");
		for (const auto& map_it : maps_)
			f(map_it.second.get());
	}

public slots:

	/**
	* @brief Get the current selected map
	* @return the selected map
	*/
	MapHandlerGen* get_selected_map() const;

	/**
	* @brief Set the current selected map
	* @param name name of the map to be selected
	*/
	void set_selected_map(const QString& name);

	// notify that current selected map has changed
	inline void notify_selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur) { emit(selected_map_changed(old, cur)); }

//	/**
//	* @brief Get the current selected tab orbit in control dock map tab interface
//	* @return 0:Dart / 1:Vertex / 2:Edge / 3:Face / 4:Volume
//	*/
//	unsigned int get_current_orbit() const;

//	/**
//	* @brief Get cell selector
//	* @param orbit Orbit (0:Dart / 1:Vertex / 2:Edge / 3:Face / 4:Volume)
//	*/
//	CellSelectorGen* get_selected_selector(unsigned int orbit) const;

//	/**
//	* @brief Set the selector of the current map (warning change the current orbit)
//	* @param orbit the orbit (0:Dart / 1:Vertex / 2:Edge / 3:Face / 4:Volume)
//	* @param name name of selector (must exist)
//	*/
//	void set_selected_selector_current_map(unsigned int orbit, const QString& name);

//	// notify that current selected cell-selection has changed
//	void notify_selected_cell_selector_changed(CellSelectorGen* cs) { emit(selected_cell_selector_changed(cs)); }


	/*********************************************************
	 * MANAGE VIEWS
	 *********************************************************/

	/**
	* @brief add a view with name
	* @param name name of view
	*/
	View* add_view(const QString& name);

	/**
	* @brief add a view
	*/
	View* add_view();

	/**
	* @brief remove a view
	* @param name the name of the view
	*/
	void remove_view(const QString& name);

	/**
	* @brief get view object
	* @param name the name of view
	*/
	View* get_view(const QString& name) const;

public:

	template <typename FUNC>
	void foreach_view(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, View*>::value, "Wrong function parameter type");
		for (const auto& view_it : views_)
			f(view_it.second.get());
	}

public slots:

	/**
	* @brief selected view cycle thru the views 
	*/
	void cycle_selected_view();

	/**
	* @brief get the selected view
	*/
	inline View* get_selected_view() const { return selected_view_; }

	/**
	* @brief set the selected view
	* @param view the view object
	*/
	void set_selected_view(View* view);

	/**
	* @brief set the selected view
	* @param name the view name
	*/
	void set_selected_view(const QString& name);

	/**
	* @brief split the view in the current orientation
	* @param orientation of split 0: Vertical Split 1:
	* @return the new View added by the split
	*/
	View* split_view(const QString& name, Qt::Orientation orientation);

	/**
	* @brief save all split positions in a string
	* @return the storage string
	*/
	QString get_split_view_positions();

	/**
	* @brief restore all split positions from a string storage,
	* the split's sequence must be the same than when saving
	*/
	void set_split_view_positions(QString positions);

	/*********************************************************
	 * MANAGE MENU ACTIONS
	 *********************************************************/

	/**
	 * @brief add an entry in menu
	 * @param menuPath path of menu (ex: "Surface; Import Mesh")
	 * @param action action to associate with entry
	 */
	QAction* add_menu_action(const QString& menu_path, const QString& action_text);

	/**
	 * @brief remove an entry in the menu
	 * @param action action entry to remove
	 */
	void remove_menu_action(QAction* action);

	/*********************************************************
	 * MANAGE WINDOW
	 *********************************************************/

	/**
	* @brief Print a message in the status bar
	* @param msg the message
	* @param msec number of milli-second that message stay printed
	*/
	void status_bar_message(const QString& msg, int msec);

	/**
	* @brief Set the window size
	* @param w width of window
	* @param h height of window
	*/
	void set_window_size(int w, int h);

	void close_window();
	void schnapps_window_closing();
	SCHNAppsWindow* get_window() { return window_; }


public slots:
	void export_settings();

signals:

	void camera_added(Camera*);
	void camera_removed(Camera*);

	void plugin_available_added(QString name);
	void plugin_enabled(Plugin* plugin);
	void plugin_disabled(Plugin* plugin);

	void map_added(MapHandlerGen*);
	void map_removed(MapHandlerGen*);
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);

	void view_added(View*);
	void view_removed(View*);
	void selected_view_changed(View*, View*);
	void view_split(View*);

	void schnapps_closing();

public:

	inline const QVariant get_setting(const QString& module_name, const QString& name) const
	{
		return settings_->get_setting(module_name, name);
	}

	inline QVariant add_setting(const QString& module_name, const QString& name, const QVariant& val)
	{
		return settings_->add_setting(module_name,name,val);
	}

	inline const QVariant get_core_setting(const QString& name) const
	{
		return settings_->get_setting("core", name);
	}

protected:

	QString app_path_;
	QString settings_path_;

	std::unique_ptr<Settings> settings_;
	std::unique_ptr<StatusBarOutput> status_bar_output_;

	std::map<QString, std::unique_ptr<Camera>> cameras_;

	std::map<QString, std::unique_ptr<Plugin>> plugins_;
	std::map<QString, QString> available_plugins_;
	std::map<Plugin*, std::list<QAction*> > plugin_menu_actions_;
	std::map<Plugin*, std::list<QWidget*> > plugin_dock_tabs_;

	std::map<QString, std::unique_ptr<MapHandlerGen>> maps_;

	std::map<QString, std::unique_ptr<View>> views_;
	View* first_view_;
	View* selected_view_;

	SCHNAppsWindow* window_;

	ControlDock_CameraTab* control_camera_tab_;
	ControlDock_PluginTab* control_plugin_tab_;
	ControlDock_MapTab* control_map_tab_;

	QSplitter* root_splitter_;
	bool root_splitter_initialized_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SCHNAPPS_H_
